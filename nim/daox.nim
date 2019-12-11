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
import dao_base
import dao_errors
import reader

const
  DEBUG = true
  FILE_SYMBOLIC = ".dao"
  FILE_COMPILED = ".wuwei"

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
        else: Unreachable("Impossible Dnk8 pow")
      of DnkNEG, DnkPOS: discard
    result.add(")")

  proc `$`(reader: ptr Reader): string =
    case reader.mode:
      of RmNODE: fmt"Read[{$reader.node}]"
      of RmPOS: fmt"Read(p{reader.pow}, i{reader.idx})[{$reader.node}]"

## Utility

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

## Operation implementation

proc swaps(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8: return
  swapsReader(EXEC.data)

proc later(EXEC: var Executor) =
  if EXEC.oplevel < 4'u8:
    ## Ordinary LATER
    laterReader(EXEC.data)
  else:
    Todo("Linear Traversal")

proc merge(EXEC: var Executor) =
  if EXEC.oplevel > 6'u8: return
  ## Ascend EXEC.data
  mergeReader(EXEC.data)

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

  # Exit if the exec reader would be crushed
  if EXEC.data.path.dataRoot.pow < 3: return

  # Copy the reader and navigate to the leftmost pow3 position in selection
  var reader = EXEC.data

  case reader.mode:
  of RmPOS:
    while reader.pow < 3:
      mergeReader(reader)
    if reader.pow == reader.node.pow:
      reader.mode = RmNODE
  of RmNODE:
    if reader.node.pow > 3:
      while reader.node.pow > 3:
        halveReader(reader)
    elif reader.pow == 3:
      discard
    else:
      Unreachable("EXECS: Reader pow was < 3 despite being RmNODE")

  # Store the old reader
  STACK.add(EXEC)

  # If child of target path is nil, create a new child
  let child = if EXEC.data.path.child == nil:
    initPath(parent = EXEC.data.path)
  else:
    EXEC.data.path.child

  # Create a new executor, cloning the current reader for its position
  var dataReader = Reader(path: child, node: child.dataRoot, mode: RmPOS, pow: 0, idx: 0)
  var newExec = Executor(oplevel: 0, execStart: reader, exec: reader, data: dataReader)

  # Swap out the executor
  EXEC = newExec

proc delev(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8:
    dec EXEC.oplevel

# proc equal() # TODO

proc halve(EXEC: var Executor) =
  ## Descend into node left half.
  if EXEC.oplevel > 6'u8: return
  halveReader(EXEC.data)

# proc uplev() # TODO

proc reads(EXEC: var Executor) =
  if EXEC.oplevel > 5'u8: return
  readsReader(EXEC.data)

# proc dealc() # TODO

proc split(EXEC: var Executor) =
  if EXEC.oplevel > 6'u8: return
  if EXEC.oplevel == 0'u8:
    splitReader(EXEC.data)
  halve(EXEC)

# proc polar() # TODO

proc doalc(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8: return
  # Allocation is performed on the root of the reader's node, not the executing path.
  # Create parent
  doalcReader(EXEC.data)

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
    let val = (hexData[i] shl 4) or hexData[i+1]
    nodes.addLast(DaoNode(kind: Dnk8, pow: 3, val: val, parent: nil))

  if hexData.len mod 2 == 1:
    # Take care of remainder if it exists.
    nodes.addLast DaoNode(kind: Dnk8, pow: 3, val: hexData[^1] shl 4)

  let execStart = nodes[0]

  # Treeify.
  while nodes.len > 1:
    var lc = nodes.popFirst()
    let rc = nodes.popFirst()
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
        nodes.addLast DaoNode(kind: DnkNEG, pow: lc.pow + 1)
      elif (lc.kind == DnkPOS or (lc.kind == Dnk8 and lc.val == 0xFF'u8)) and
           (rc.kind == DnkPOS or (rc.kind == Dnk8 and rc.val == 0xFF'u8)):
        nodes.addLast DaoNode(kind: DnkPOS, pow: lc.pow + 1)
      else:
        let parent = DaoNode(kind: DnkMIX, pow: lc.pow + 1, parent: nil, lc: lc, rc: rc)
        lc.parent = parent
        rc.parent = parent
        nodes.addLast parent

  # Now we have made the tree for the TLP.
  var tlp = initPath(dataRoot = nodes.popFirst())

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

  # Initialize program stack and base executor
  var STACK: seq[Executor]
  var EXEC = hexData.initTLP

  doalc(EXEC)
  doalc(EXEC)
  doalc(EXEC)
  
  echo "Exec0: ", EXEC.exec

  execs(EXEC, STACK)

  echo "Exec0: ", EXEC.exec
  echo "Exec1: ", STACK[0].exec


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
      of '#': EXEC.execs(STACK)
      of '>': EXEC.delev
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
