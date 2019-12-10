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
    case kind: DaoNodeKind
      of DnkMIX:
        lc, rc: DaoNode
        # is_equal, is_polar: bool
      of Dnk8:
        val: uint8
      of DnkNEG, DnkPOS: discard

  ReaderMode = enum
    RmNODE                     ##  On exterior, at node level
    RmPOS                      ##  On interior of uint8 node (8/4/2) or uniform node (POS/NEG)

  Reader = object
    ## The reader reads instructions from the exec node
    ##   and operates on the Data node.
    root:         DaoNode      ##  The root of the path operated on by the Reader
    ## Exec node - typically points to owner's parent Path
    oplevel:      uint8        ##  Operating level
    exec:         DaoNode      ##  Location of execution node
    ## Data node - typically points to owner Path
    node:         DaoNode      ##  Location of data node
    mode:         ReaderMode   ##  If in the interior of a complex node
    pow:          uint64       ##  record pow2
    idx:          uint64       ##  and index from left at that resolution

  Path = ref object
    owner, child: Path         ##  Owner and Child programs (may be nil)
    depth:        uint64       ##  How deep is this in the program tree?
    root:         DaoNode      ##  Points to the root of the data
    exec_start:   DaoNode      ##  Where we started running
    reader:       Reader       ##  Program Pointer operating ON this path.

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
      of RmNODE: fmt"Read<{reader.oplevel}>[{$reader.node}]"
      of RmPOS: fmt"Read<{reader.oplevel}>(p{reader.pow}, i{reader.idx})[{$reader.node}]"

proc initPath(): Path =
  ## Initialize a new path as the top level program.
  result.new
  result.owner = nil
  result.child = nil
  result.depth = 0
  result.root = DaoNode(kind: Dnk8, parent: nil, pow: 0, val: 0'u8)
  result.exec_start = nil
  result.reader = Reader(root: result.root, node: result.root, mode: RmPOS, pow: 0, idx: 0, oplevel: 0)

## Functions

proc swaps(p: Path)
proc later(p: Path)
proc merge(p: Path)
# proc sifts(p: Path)
# proc execs(p: Path)
proc delev(p: Path)
# proc equal(p: Path)
proc halve(p: Path)
# proc uplev(p: Path)
proc reads(p: Path)
# proc dealc(p: Path)
# proc split(p: Path)
# proc polar(p: Path)
proc doalc(p: Path)

proc swaps(p: Path) =
  if p.reader.oplevel > 0'u8: return
  let node = p.reader.node
  case node.kind:
    of DnkMIX: swap(node.lc, node.rc)
    of Dnk8:
      if p.reader.pow == 3: # RmNODE
        node.val = (node.val shl 4 and 0xF0'u8) or (node.val shr 4)
      elif p.reader.pow == 2: # RmPOS
        let bitsFromRight = 4 * (1 - p.reader.idx)
        let writeMask = 0b1111'u8 shl bitsFromRight
        let shifted = (node.val shl 2 and 0b11001100'u8) or (node.val shr 2 and 0b00110011'u8)
        node.val = (node.val and not writeMask) or (shifted and writeMask)
      elif p.reader.pow == 1: # RmPOS
        let bitsFromRight = 2 * (3 - p.reader.idx)
        let writeMask = 0b11'u8 shl bitsFromRight
        let shifted = (node.val shl 1 and 0b10101010'u8) or (node.val shr 1 and 0b01010101'u8)
        node.val = (node.val and not writeMask) or (shifted and writeMask)
      elif node.pow == 0: # RmPOS, swap bit is discard
        discard
      else:
        Unreachable("SWAPS: Impossible Dnk8 pow.")
    else: discard

proc later(p: Path) =
  if p.reader.oplevel < 4'u8:
    ## Ordinary LATER
    case p.reader.mode:
    of RmNODE:
      if p.reader.node.parent == nil or p.reader.node == p.reader.node.parent.rc:
        p.merge
      else:
        if p.reader.node == p.reader.node.parent.lc:
          p.reader.node = p.reader.node.parent.rc
        else:
          Unreachable("LATER: Unchained node.")
    of RmPOS:
      if (p.reader.idx and 1'u8) != 0:
        # If on the second virtual half of some node, merge
        p.merge
      else:
        # Otherwise, go to the right half
        inc p.reader.idx
  else:
    Todo("Linear Traversal")

proc merge(p: Path) =
  if p.reader.oplevel > 6'u8: return
  ## Ascend reader
  case p.reader.mode:
    of RmNODE:
      if p.reader.node.parent != nil:
        p.reader.node = p.reader.node.parent
    of RmPOS:
      if p.reader.pow == p.reader.node.pow:
        # Only should happen when inside a Dnk8 of pow < 3.
        if p.reader.node.parent == nil:
          # We should be at the top.
          if p.owner != nil:
            # Try to go to the first BIT of the upper program.
            Todo("Select first bit of upper program.")
        else:
          Unreachable("MERGE: RmPOS Pow match but parent is not nil.")
      else:
        p.reader.pow += 1
        p.reader.idx = p.reader.idx div 2
        if p.reader.pow > 2 and p.reader.pow == p.reader.node.pow:
          p.reader.mode = RmNODE

# proc sifts(p: Path) # TODO
# proc execs(p: Path; a3: Path) # TODO

proc delev(p: Path) =
  if p.reader.oplevel > 0'u8:
    dec p.reader.oplevel

# proc equal(p: Path) # TODO

proc halve(p: Path) =
  ## Descend into node left half.
  if p.reader.oplevel > 6'u8: return
  let node = p.reader.node
  case p.reader.mode:
  of RmNODE:
    case node.kind:
      of DnkMIX:
        p.reader.node = node.lc
      of Dnk8, DnkPOS, DnkNEG:
        if node.pow == 0 and p.child != nil:
          ## Descend into bit -> Select child path
          p.reader.node = p.child.root
          p.reader.root = p.child.root
          p.reader.mode = RmNODE
        else:
          p.reader.pow = node.pow - 1
          p.reader.idx = 0
          p.reader.mode = RmPOS
  of RmPOS:
    if p.reader.pow == 0:
      ## Descend into bit -> Select child path
      if p.child != nil:
        p.reader.node = p.child.root
        p.reader.root = p.child.root
        p.reader.mode = RmNODE
    else:
      dec p.reader.pow
      p.reader.idx = p.reader.idx shl 1

# proc uplev(p: Path) # TODO

proc reads(p: Path) =
  if p.reader.oplevel > 5'u8: return
  let node = p.reader.node
  case p.reader.mode:
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
        for _ in p.reader.idx ..< (1'u64 shl (p.reader.pow - 3)):
          stdout.write val
      of Dnk8:
        if p.reader.pow == 0:   stdout.write "01"[node.val shr (8 - p.reader.idx) and 1'u8]
        elif p.reader.pow == 1: stdout.write (node.val shr (4 - p.reader.idx) and 0b11'u8).toBin2
        elif p.reader.pow == 2: stdout.write (node.val shr (2 - p.reader.idx) and 0xF'u8).toBin4
        else: Unreachable("READS: Impossible Reader pow.")

# proc dealc(p: Path) # TODO

proc split(p: Path) =
  if p.reader.oplevel > 6'u8: return
  if p.reader.oplevel == 0'u8:
    let node = p.reader.node
    case p.reader.mode:
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
        p.reader.node = newNode
      of Dnk8:
        node.val = 0xF0'u8
    of RmPOS:
      case node.kind:
        of Dnk8:
          if p.reader.pow == 2:
            node.val = node.val and (0x0F'u8 shl (4 * p.reader.idx))
            node.val = node.val or (0b1100'u8 shl (4 * (1 - p.reader.idx)))
          elif p.reader.pow == 1:
            let bitsFromRight = 2 * (3 - p.reader.idx)
            node.val = node.val and not (0b11'u8 shl bitsFromRight)
            node.val = node.val or (0b10'u8 shl bitsFromRight)
          elif p.reader.pow == 0:
            discard # Splitting a bit is a noop
          else:
            Unreachable("SPLIT: Impossible Reader pow.")
        of DnkNEG:
          Todo("Heterogenize DnkNEG Interior")
        of DnkPOS:
          Todo("Heterogenize DnkPOS Interior")
        of DnkMIX: Unreachable("SPLIT: POS mode inside DnkMIX")
  p.halve


# proc polar(p: Path) # TODO

proc doalc(p: Path) =
  if p.reader.oplevel > 0'u8: return
  # Allocation is performed on the root of the reader's node, not the executing path.
  # Create parent
  let root = p.reader.root
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
    p.reader.root = parent
  p.merge

# proc input(p: Path) # TODO

when isMainModule:

  var path = initPath()
  const prg = "$$$(([]!)/([])):((/[])/([]!/[]!)):(/[])::[/([]!/[])]!:[[[]]!]:[([]!)/[/[]!]!]:[/([]!/[])]!:[([]!)/(/[])]:((/[])/[]):(/([]!)):([[]]!/[[]!]!):[[[]/[]]]!:"

  for c in prg:
    # echo c, " on ", path.reader.addr
    case c:
      of '.': discard
      of '!': path.swaps
      of '/': path.later
      of ']', ')': path.merge
      # of '%': path.sifts
      # of '#': path.execs
      # of '>': path.delev
      # of '=': path.equal
      of '(': path.halve
      # of '<': path.uplev
      of ':': path.reads
      # of 'S': path.dealc
      of '[': path.split
      # of '*': path.polar
      of '$': path.doalc
      # of ';': path.input
      else: discard
    # echo c, " to ", path.reader.addr

  # path.swaps
  # path.reads
  # echo "\n", path.reader.addr

  # path.doalc
  # path.reads
  # echo "\n", path.reader.addr

  # path.doalc
  # path.reads
  # echo "\n", path.reader.addr

  # path.doalc
  # path.reads
  # echo "\n", path.reader.addr

  # path.doalc
  # path.reads
  # echo "\n", path.reader.addr

  # path.doalc
  # path.reads
  # echo "\n", path.reader.addr
