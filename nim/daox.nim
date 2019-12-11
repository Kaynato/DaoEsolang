## 
##    This program is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
## 
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
## 
##    You should have received a copy of the GNU General Public License
##    along with this program.  If not, see <http://www.gnu.org/licenses/>.
## 
## 
##  DaoLanguage / Daoyu Compiler and Interpreter. (Nim)
##
##  Lazy Daoyu, as it was meant to be.
##  (Well, more accurately, hybrid)
##  Kaynato - 2019
## 

# import bitops
import deques
from util import toBin2, toBin4

const
  DEBUG = true
  FILE_SYMBOLIC = ".dao"
  FILE_COMPILED = ".wuwei"

type
  DaoNodeKind = enum
    DnkNEG, DnkPOS            ##  Pure nodes of greater than 8 bits
    DnkMIX                    ##  Node of greater than 8 bits with heterogeneous children
    Dnk8                      ##  Mixed node of 8 or fewer bits.

  DaoNode = ref object
    parent: DaoNode
    pow: uint64               ##  2^pow bits "contained" by this node.
    p, n: bool                ##  Front and last bit of node
    case kind: DaoNodeKind
      of DnkMIX:
        lc, rc: DaoNode
      of Dnk8:
        val: uint8
      of DnkNEG, DnkPOS: discard

  ReaderMode = enum
    RmNODE                     ##  On exterior, at node level
    RmPOS                      ##  On interior of uint8 node (8/4/2) or uniform node (POS/NEG)

  Reader = object
    ## The reader allows for traversal of DaoNodes.
    path:         Path         ##  The path operated on by the Reader
    node:         DaoNode      ##  Location of data node
    mode:         ReaderMode   ##  If in the interior of a complex node
    pow:          uint64       ##  record pow2
    idx:          uint64       ##  and index from left at that resolution

  Executor = object
    ## The executor reads instructions from a tracked location in the exec node
    ##     and operates on the data node.
    oplevel:      uint8        ##  Operating level
    execStart:    Reader       ##  Location where execution began (return to when UPLEV)
    exec:         Reader       ##  Tracker for execution
    data:         Reader       ##  Tracker for target of execution

  Path = ref object
    owner, child: Path         ##  Owner and Child programs (may be nil)
    depth:        uint64       ##  How deep is this in the program tree?
    dataRoot:     DaoNode      ##  Points to the root of the data owned by this path

  UnreachableError = object of Exception
  NotImplementedError = object of Exception

template Unreachable(msg: string) =
  raise UnreachableError.newException(msg)

template Todo(msg: string) =
  raise NotImplementedError.newException(msg)

when DEBUG:
  import strformat
  from strutils import toHex, toBin

  proc `$`(node: DaoNode): string =
    if node == nil:
      Unreachable("DEBUG: Tried to print nil node")
    result.add($(node.kind))
    let treePos = (if node.parent != nil: "CHLD" else: "ROOT")
    result.add(fmt"({treePos}: pow: {node.pow}")
    case node.kind:
      of DnkMIX:
        result.add(fmt", lc: {node.lc}, rc: {node.rc}")
      of Dnk8:
        if node.pow == 3:   result.add(fmt", val: x{node.val.toHex}")
        elif node.pow == 2: result.add(fmt", val: x{node.val.toHex[1]}")
        elif node.pow == 1: result.add(fmt", val: b{node.val.BiggestInt.toBin(2)}")
        elif node.pow == 0: result.add(fmt", val: b" & ("01"[node.val]))
        else: raise UnreachableError.newException("Impossible Dnk8 pow")
      of DnkNEG, DnkPOS: discard
    result.add(")")

  proc `$`(reader: ptr Reader): string =
    case reader.mode:
      of RmNODE: fmt"Read[{$reader.node}]"
      of RmPOS: fmt"Read(p{reader.pow}, i{reader.idx})[{$reader.node}]"

proc initPath(parent: Path = nil, dataRoot: DaoNode = nil): Path =
  ## Initialize a new path from a parent, or if parentless, the TLP.
  result.new
  result.owner = parent
  result.child = nil
  if parent != nil:
    result.depth = parent.depth + 1
    parent.child = result
  else:
    result.depth = 0
  if dataRoot == nil: result.dataRoot = DaoNode(kind: Dnk8, parent: nil, pow: 0, val: 0'u8)
  else:               result.dataRoot = dataRoot

## Functions

proc swaps(EXEC: var Executor)
proc later(EXEC: var Executor)
proc merge(EXEC: var Executor)
# proc sifts(EXEC: var Executor)
proc execs(EXEC: var Executor, STACK: var seq[Executor])
proc delev(EXEC: var Executor)
# proc equal(EXEC: var Executor)
proc halve(EXEC: var Executor)
# proc uplev(EXEC: var Executor)
proc reads(EXEC: var Executor)
# proc dealc(EXEC: var Executor)
# proc split(EXEC: var Executor)
# proc polar(EXEC: var Executor)
proc doalc(EXEC: var Executor)

proc swaps(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8: return
  let node = EXEC.data.node
  case node.kind:
    of DnkMIX: swap(node.lc, node.rc)
    of Dnk8:
      if EXEC.data.pow == 3: # RmNODE
        node.val = (node.val shl 4 and 0xF0'u8) or (node.val shr 4)
      elif EXEC.data.pow == 2: # RmPOS
        let bitsFromRight = 4 * (1 - EXEC.data.idx)
        let writeMask = 0b1111'u8 shl bitsFromRight
        let shifted = (node.val shl 2 and 0b11001100'u8) or (node.val shr 2 and 0b00110011'u8)
        node.val = (node.val and not writeMask) or (shifted and writeMask)
      elif EXEC.data.pow == 1: # RmPOS
        let bitsFromRight = 2 * (3 - EXEC.data.idx)
        let writeMask = 0b11'u8 shl bitsFromRight
        let shifted = (node.val shl 1 and 0b10101010'u8) or (node.val shr 1 and 0b01010101'u8)
        node.val = (node.val and not writeMask) or (shifted and writeMask)
      elif node.pow == 0: # RmPOS, swap bit is discard
        discard
      else:
        Unreachable("SWAPS: Impossible Dnk8 pow.")
    else: discard

proc later(EXEC: var Executor) =
  if EXEC.oplevel < 4'u8:
    ## Ordinary LATER
    case EXEC.data.mode:
    of RmNODE:
      if EXEC.data.node.parent == nil or EXEC.data.node == EXEC.data.node.parent.rc:
        merge(EXEC)
      else:
        if EXEC.data.node == EXEC.data.node.parent.lc:
          EXEC.data.node = EXEC.data.node.parent.rc
        else:
          Unreachable("LATER: Unchained node.")
    of RmPOS:
      # If on the second virtual half of some node, merge
      if (EXEC.data.idx and 1'u8) != 0: merge(EXEC)
      # Otherwise, go to the right half
      else: inc EXEC.data.idx
  else:
    Todo("Linear Traversal")

proc merge(EXEC: var Executor) =
  if EXEC.oplevel > 6'u8: return
  ## Ascend EXEC.data
  case EXEC.data.mode:
    of RmNODE:
      if EXEC.data.node.parent != nil:
        EXEC.data.node = EXEC.data.node.parent
    of RmPOS:
      if EXEC.data.pow == EXEC.data.node.pow:
        # Only should happen when inside a Dnk8 of pow < 3.
        if EXEC.data.node.parent == nil:
          # We should be at the top.
          if EXEC.data.path.owner != nil:
            # Try to go to the first BIT of the upper program.
            Todo("Select first bit of upper program.")
        else:
          Unreachable("MERGE: RmPOS Pow match but parent is not nil.")
      else:
        EXEC.data.pow += 1
        EXEC.data.idx = EXEC.data.idx div 2
        if EXEC.data.pow > 2 and EXEC.data.pow == EXEC.data.node.pow:
          EXEC.data.mode = RmNODE

# proc sifts() # TODO

#[ 
  When execs,
    Push the current LIVE_READER (Start Location, Instruction Reader, Data Path) to the storage stack.
    Create a new Reader for LIVE_READER:
      new start location = leftmost pow 3 node at instruction reader / round up
        if cannot round up, the instruction pointer is "crushed" and execs is nop
      instruction reader is cloned from the current Reader
        and navigates to the new start location
      data path is the CHILD of the path which has the data the instruction reader points to.

  Something to think about is
    when you call EXECS such that
    the data path is an existing path, in which case... dangers arise

  When a program finishes, should pop from LIVE_STACK. If empty, terminate.

# TODO
  Consider making "path in node" where each call of EXECS produces a new Path....

]#

proc execs(EXEC: var Executor, STACK: var seq[Executor]) =
  if EXEC.oplevel > 7'u8: return
  # Store the old reader
  STACK.add(EXEC)

  # If child of target path is nil, create a new child
  if EXEC.data.path.child == nil:
    let child = initPath(parent = EXEC.data.path)

  # Copy the reader and navigate to the correct location
  var reader = EXEC.data
  # TODO

  # Create a new executor, cloning the current reader for its position
  var newExec = Executor(oplevel: 0, )
  # child.reader = Reader(path: child, node: child.dataRoot, mode: RmPOS, pow: 0, idx: 0, oplevel: 0)

proc delev(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8:
    dec EXEC.oplevel

# proc equal() # TODO

proc halve(EXEC: var Executor) =
  ## Descend into node left half.
  if EXEC.oplevel > 6'u8: return
  let node = EXEC.data.node
  case EXEC.data.mode:
  of RmNODE:
    case node.kind:
      of DnkMIX:
        EXEC.data.node = node.lc
      of Dnk8, DnkPOS, DnkNEG:
        let child = EXEC.data.path.child
        if node.pow == 0 and child != nil:
          ## Descend into bit -> Select child path
          EXEC.data.node = child.dataRoot
          EXEC.data.path = child
          EXEC.data.mode = RmNODE
        else:
          EXEC.data.pow = node.pow - 1
          EXEC.data.idx = 0
          EXEC.data.mode = RmPOS
  of RmPOS:
    if EXEC.data.pow == 0:
      ## Descend into bit -> Select child path
      let child = EXEC.data.path.child
      if child != nil:
        EXEC.data.node = child.dataRoot
        EXEC.data.path = child
        EXEC.data.mode = RmNODE
    else:
      dec EXEC.data.pow
      EXEC.data.idx = EXEC.data.idx shl 1

# proc uplev() # TODO

proc reads(EXEC: var Executor) =
  if EXEC.oplevel > 5'u8: return
  let node = EXEC.data.node
  case EXEC.data.mode:
  of RmNODE:
    case node.kind:
      of DnkMIX:
        echo "Need linear traversal to be implemented."
        raise new LibraryError
      of DnkNEG, DnkPOS:
        let val = (if node.kind == DnkPOS: 0xFF.char else: 0x00.char)
        for _ in 0 ..< (1'u64 shl (node.pow - 3)):
          stdout.write val
      of Dnk8:
        if node.pow == 3:   stdout.write node.val.char
        elif node.pow == 2: stdout.write node.val.toBin4
        elif node.pow == 1: stdout.write node.val.toBin2
        elif node.pow == 0: stdout.write "01"[node.val]
        else: Unreachable("READS: Impossible Dnk8 pow.")
  of RmPOS:
    case node.kind:
      of DnkMIX:
        Unreachable("READS: Accessed DnkMIX in POS mode.")
      of DnkNEG, DnkPOS:
        let val = (if node.kind == DnkPOS: 0xFF.char else: 0x00.char)
        for _ in EXEC.data.idx ..< (1'u64 shl (EXEC.data.pow - 3)):
          stdout.write val
      of Dnk8:
        if EXEC.data.pow == 0:   stdout.write "01"[node.val shr (8 - EXEC.data.idx) and 1'u8]
        elif EXEC.data.pow == 1: stdout.write (node.val shr (4 - EXEC.data.idx) and 0b11'u8).toBin2
        elif EXEC.data.pow == 2: stdout.write (node.val shr (2 - EXEC.data.idx) and 0xF'u8).toBin4
        else: Unreachable("READS: Impossible EXEC.Reader pow.")

# proc dealc() # TODO

proc split(EXEC: var Executor) =
  if EXEC.oplevel > 6'u8: return
  if EXEC.oplevel == 0'u8:
    let node = EXEC.data.node
    case EXEC.data.mode:
    of RmNODE:
      case node.kind:
      of DnkMIX:
        if node.pow > 4:
          if node.lc.kind != DnkPOS: node.lc = DaoNode(kind: DnkPOS, parent: node, pow: node.pow - 1)
          if node.rc.kind != DnkNEG: node.rc = DaoNode(kind: DnkNEG, parent: node, pow: node.pow - 1)
        elif node.pow == 4:
          if node.lc.kind != DnkPOS: node.lc = DaoNode(kind: Dnk8, parent: node, pow: 3, val: 0xFF'u8)
          if node.rc.kind != DnkNEG: node.rc = DaoNode(kind: Dnk8, parent: node, pow: 3, val: 0x00'u8)
        else: Unreachable("SPLIT: Illegal DnkMIX of pow <= 3")
      of DnkNEG, DnkPOS:
        # Overwrite the node
        let newNode = DaoNode(kind: DnkMIX, parent: node.parent, pow: node.pow, lc: nil, rc: nil)
        if node.pow > 4:
          newNode.lc = DaoNode(kind: DnkPOS, parent: newNode, pow: node.pow - 1)
          newNode.rc = DaoNode(kind: DnkNEG, parent: newNode, pow: node.pow - 1)
        elif node.pow == 4:
          newNode.lc = DaoNode(kind: Dnk8, parent: node, pow: 3, val: 0xFF'u8)
          newNode.rc = DaoNode(kind: Dnk8, parent: node, pow: 3, val: 0x00'u8)
        else: Unreachable("SPLIT: Illegal DnkMIX of pow <= 3")
        EXEC.data.node = newNode
      of Dnk8:
        node.val = 0xF0'u8
    of RmPOS:
      case node.kind:
        of Dnk8:
          if EXEC.data.pow == 2:
            node.val = node.val and (0x0F'u8 shl (4 * EXEC.data.idx))
            node.val = node.val or (0b1100'u8 shl (4 * (1 - EXEC.data.idx)))
          elif EXEC.data.pow == 1:
            let bitsFromRight = 2 * (3 - EXEC.data.idx)
            node.val = node.val and not (0b11'u8 shl bitsFromRight)
            node.val = node.val or (0b10'u8 shl bitsFromRight)
          elif EXEC.data.pow == 0:
            discard # Splitting a bit is a noop
          else:
            Unreachable("SPLIT: Impossible Reader pow.")
        of DnkNEG:
          Todo("Heterogenize DnkNEG Interior")
        of DnkPOS:
          Todo("Heterogenize DnkPOS Interior")
        of DnkMIX: Unreachable("SPLIT: POS mode inside DnkMIX")
  halve(EXEC)


# proc polar() # TODO

proc doalc(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8: return
  # Allocation is performed on the root of the reader's node, not the executing path.
  # Create parent
  let root = EXEC.data.path.dataRoot
  let parent = case root.kind:
    of DnkMIX, DnkPOS:
      DaoNode(kind: DnkMIX, pow: root.pow + 1, parent: nil, lc: root, rc: DaoNode(kind: DnkNEG, pow: root.pow, parent: nil))
    of Dnk8:
      if root.pow == 3:
        if root.val == 0'u8:
          DaoNode(kind: DnkNEG, pow: 4, parent: nil)
        else:
          DaoNode(kind: DnkMIX, lc: root, pow: 4, parent: nil, rc: DaoNode(kind: Dnk8, pow: 3, parent: nil, val: 0'u8))
      elif root.pow < 3:
        ## Special case: Just modify the Dnk8 instead.
        root.val = root.val shl (1 shl root.pow)
        root.pow.inc(1)
        root
      else: Unreachable("DOALC: Impossible Dnk8 pow.")
    of DnkNEG: DaoNode(kind: DnkNEG, pow: root.pow + 1, parent: nil)
  # Bind parents ONLY if it's not the special modification case
  if parent.kind != Dnk8:
    if parent.kind == DnkMIX:
      parent.rc.parent = parent
    root.parent = parent
    EXEC.data.path.dataRoot = parent
  merge(EXEC)

# proc input() # TODO

proc symToVal(ch: char): uint8 =
  ## Convert symbols into bytes
  case ch:
    of '.': 0x00'u8
    of '!': 0x01'u8
    of '/': 0x02'u8
    of ']', ')': 0x03'u8
    of '%': 0x04'u8
    of '#': 0x05'u8
    of '>': 0x06'u8
    of '=': 0x07'u8
    of '(': 0x08'u8
    of '<': 0x09'u8
    of ':': 0x0A'u8
    of 'S': 0x0B'u8
    of '[': 0x0C'u8
    of '*': 0x0D'u8
    of '$': 0x0E'u8
    of ';': 0x0F'u8
    else: 0x00'u8

proc initTLP(hexData: seq[uint8]): Executor =
  ## Convert seq of hexes into an equivalent Path
  if hexData.len == 0: return

  var nodes = initDeque[DaoNode]()

  for i in countup(0, ((hexData.len + 1) div 2) - 1, 2):
    let val = hexData[i] shl 4 or hexData[i+1]
    nodes.addFirst(DaoNode(kind: Dnk8, pow: 3, val: val, parent: nil))

  if hexData.len mod 2 == 1:
    # Take care of remainder if it exists.
    nodes.addFirst DaoNode(kind: Dnk8, pow: 3, val: hexData[^1] shl 4)

  let execStart = nodes[0]

  # Treeify.
  while nodes.len > 1:
    var lc = nodes.popLast()
    let rc = nodes.popLast()
    while lc.pow < rc.pow:
      # Need zero padding on right
      if (lc.kind == DnkNEG or (lc.kind == Dnk8 and lc.val == 0'u8)):
        # If left is blank, just escalate
        inc lc.pow
      else:
        # Otherwise, replace as lc of new mix parent with empty rc
        lc = DaoNode(kind: DnkMIX, pow: lc.pow + 1, lc: lc)
        lc.rc = DaoNode(kind: DnkNEG, pow: lc.pow - 1)
    if lc.pow > rc.pow:
      Unreachable("initTLP: ILLEGAL - First child had greater pow than second.")
    else:
      # Equal power
      if (lc.kind == DnkNEG or (lc.kind == Dnk8 and lc.val == 0'u8)) and
         (rc.kind == DnkNEG or (rc.kind == Dnk8 and rc.val == 0'u8)):
        nodes.addFirst DaoNode(kind: DnkNEG, pow: lc.pow + 1)
      elif (lc.kind == DnkPOS or (lc.kind == Dnk8 and lc.val == 0xFF'u8)) and
           (rc.kind == DnkPOS or (rc.kind == Dnk8 and rc.val == 0xFF'u8)):
        nodes.addFirst DaoNode(kind: DnkPOS, pow: lc.pow + 1)
      else:
        let parent = DaoNode(kind: DnkMIX, pow: lc.pow + 1, parent: nil, lc: lc, rc: rc)
        lc.parent = parent
        rc.parent = parent
        nodes.addFirst parent

  # Now we have made the tree for the TLP.
  var tlp = initPath(dataRoot = nodes.popLast())

  # Special case child construction
  var child = initPath(parent = tlp)

  # Special case executor construction
  result.oplevel = 0
  result.execStart = Reader(path: tlp, mode: RmNODE, node: execStart, pow: 3, idx: 0)
  result.exec = Reader(path: tlp, mode: RmNODE, node: execStart, pow: 3, idx: 0)
  result.data = Reader(path: child, mode: RmPOS, node: child.dataRoot, pow: 0, idx: 0)

when isMainModule:

  const prg = "$$$(([]!)/([])):((/[])/([]!/[]!)):(/[])::[/([]!/[])]!:[[[]]!]:[([]!)/[/[]!]!]:[/([]!/[])]!:[([]!)/(/[])]:((/[])/[]):(/([]!)):([[]]!/[[]!]!):[[[]/[]]]!:"
  var hexData: seq[uint8]
  for c in prg:
    hexData.add c.symToVal

  var EXEC = hexData.initTLP

  # Then call path.execs...

  # var path = initPath()

  for c in prg:
    # echo c, " on ", EXEC.reader.addr
    case c:
      of '.': discard
      of '!': EXEC.swaps
      of '/': EXEC.later
      of ']', ')': EXEC.merge
      # of '%': EXEC.sifts
      # of '#': EXEC.execs
      # of '>': EXEC.delev
      # of '=': EXEC.equal
      of '(': EXEC.halve
      # of '<': EXEC.uplev
      of ':': EXEC.reads
      # of 'S': EXEC.dealc
      of '[': EXEC.split
      # of '*': EXEC.polar
      of '$': EXEC.doalc
      # of ';': EXEC.input
      else: discard
    # echo c, " to ", EXEC.reader.addr
