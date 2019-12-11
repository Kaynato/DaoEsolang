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

  # proc `$`(node: DaoNode): string =
  #   if node == nil:
  #     Unreachable("DEBUG: Tried to print nil node")
  #   result.add($(node.kind))
  #   let treePos = (if node.parent != nil: "CHLD" else: "ROOT")
  #   result.add(fmt"({treePos}: pow: {node.pow}")
  #   case node.kind:
  #     of DnkMIX:
  #       result.add(fmt", lc: {node.lc}, rc: {node.rc}")
  #     of Dnk8:
  #       if node.pow == 3:   result.add(fmt", val: x{node.val.toHex}")
  #       elif node.pow == 2: result.add(fmt", val: x{node.val.toHex[1]}")
  #       elif node.pow == 1: result.add(fmt", val: b{node.val.BiggestInt.toBin(2)}")
  #       elif node.pow == 0: result.add(fmt", val: b" & ("01"[node.val]))
  #       else: Unreachable("Impossible Dnk8 pow")
  #     of DnkNEG, DnkPOS: discard
  #   result.add(")")

  proc `$`(node: DaoNode): string =
    if node == nil:
      Unreachable("DEBUG: Tried to print nil node")
    case node.kind:
      of DnkMIX:
        result.add(fmt"({node.lc} {node.rc})")
      of Dnk8:
        if node.pow == 3:   result.add(fmt"(x{node.val.toHex})")
        elif node.pow == 2: result.add(fmt"(x{node.val.toHex[1]})")
        elif node.pow == 1: result.add(fmt"(b{node.val.BiggestInt.toBin(2)})")
        elif node.pow == 0: result.add(fmt"(b" & ("01"[node.val]) & ")")
        else: Unreachable("Impossible Dnk8 pow")
      of DnkNEG: result.add(fmt"({node.pow} 0)")
      of DnkPOS: result.add(fmt"({node.pow} 1)")

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
    discard laterReader(EXEC.data)
  else:
    Todo("Linear Traversal")

proc merge(EXEC: var Executor) =
  if EXEC.oplevel > 6'u8: return
  ## Ascend EXEC.data
  discard mergeReader(EXEC.data)

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
      discard mergeReader(reader)
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

  for i in countup(0, hexData.len - 2, 2):
    let val = (hexData[i] shl 4) or hexData[i+1]
    nodes.addLast(DaoNode(kind: Dnk8, pow: 3, val: val, parent: nil))

  if hexData.len mod 2 == 1:
    # Take care of remainder if it exists.
    nodes.addLast DaoNode(kind: Dnk8, pow: 3, val: hexData[^1] shl 4)

  let execStart = nodes[0]

  # Treeify.
  while nodes.len > 1:
    let lc = nodes.popFirst()
    let rcPeek = nodes.peekFirst()
    
    if lc.pow < rcPeek.pow:
      # Need zero padding on right
      if lc.kind == DnkNEG:
        # If left is blank, just escalate
        inc lc.pow
        nodes.addLast lc
      elif (lc.kind == Dnk8 and lc.val == 0'u8):
        # Blank dnk8
        nodes.addLast DaoNode(kind: DnkNEG, pow: lc.pow + 1)
      else:
        # Otherwise, replace as lc of new mix parent with empty rc
        let parent = DaoNode(kind: DnkMIX, pow: lc.pow + 1, lc: lc)
        if parent.pow > 3:
          parent.rc = DaoNode(kind: DnkNEG, pow: lc.pow)
        elif parent.pow == 3:
          parent.rc = DaoNode(kind: Dnk8, pow: 3, val: 0'u8)
        else:
          Unreachable("initTLP: ILLEGAL - Tried to create node with pow under 2")
        nodes.addLast parent
    elif lc.pow == rcPeek.pow:
      let rc = nodes.popFirst()
      # Equal power. Check for pure nodes first
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
    else:
      Unreachable("initTLP: ILLEGAL - First child had greater pow than second.")

  # Now we have made the tree for the TLP.
  var tlp = initPath(dataRoot = nodes.popFirst())

  # Special case child construction
  var child = initPath(parent = tlp)

  # Special case executor construction
  result.oplevel = 0
  result.execStart = Reader(path: tlp, mode: RmNODE, node: execStart, pow: 3, idx: 0)
  result.exec = Reader(path: tlp, mode: RmPOS, node: execStart, pow: 2, idx: 0)
  result.data = Reader(path: child, mode: RmPOS, node: child.dataRoot, pow: 0, idx: 0)

proc interpret(prg: string, EXEC: var Executor) =
  ## Directly interpret daoyu symbols
  # Unstable
  # TODO make this NOT reliant on EXEC being passed in please
  var STACK: seq[Executor]
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

const SIGKILL = 0xFF

proc moveThenGet(reader: var Reader, debug: static bool = false): uint8 =
  # Move to next linear position and return the value there.
  if reader.pow != 2:
    Unreachable("moveThenGet: Exec reader has non-2 pow")
  if reader.mode == RmNODE:
    Unreachable("moveThenGet: Exec reader was pow 2 but was reading Nodes")
  # We are certainly inside of DnkPOS or DnkNEG.
  let pdiff = reader.node.pow - reader.pow
  if pdiff >= 64:
    Todo("moveThenGet: Index might overflow uint64")
  # If we can move right, just move and get the value.
  if (((reader.idx + 1) shr pdiff) and 1'u64) == 0:
    # Move right and get value
    inc reader.idx
    return reader.getHex()
  # Otherwise, we have to actually merge out of the node itself
  reader.mode = RmNODE
  reader.pow = reader.node.pow
  reader.idx = 0
  # Coming out here means we need to come out of the node :(
  # No parent or was right child
  if reader.node.parent == nil:
    when debug: echo "At root, no cmd"
    return SIGKILL
  # Was right child - go up until is left child or hits root node
  while reader.node != reader.node.parent.lc:
    discard mergeReader(reader, allow_ascent=false)
    if reader.node.parent == nil:
      when debug: echo "Ascend to root, no cmd"
      return SIGKILL
    else:
      when debug: echo "Merged to find rc"
  # Now we are the left child, so move right
  if not laterReader(reader, allow_merge=false):
    Unreachable("moveThenGet: Tried to merge despite being left child.")
  # Move down until pow is 3
  while reader.pow > 2:
    halveReader(reader)
  # Now we're good
  return reader.getHex()

const OP_NAMES = [
  "IDLES", "SWAPS", "LATER", "MERGE",
  "SIFTS", "EXECS", "DELEV", "EQUAL",
  "HALVE", "UPLEV", "READS", "DEALC",
  "SPLIT", "POLAR", "DOALC", "INPUT"
]

const OP_SYM = ".!/)%#>=(<:S[*$;"

proc run(EXEC: var Executor, debug: static bool = false) =
  ## Run until termination
  var STACK: seq[Executor]
  var next_cmd: uint8 = EXEC.exec.getHex()
  while true:
    while next_cmd != 0xFF:
      when debug: echo fmt"Next_cmd was {OP_SYM[next_cmd]}"

      # I guess we can maintain a map : node -> exec readers
      # That way we can properly move them if the node changes
      # Or nil their node if DEALC'd so the gc works and you don't execute on empty code
      # Hahaha wait that means that we have to go and destroy data readers too oh no
      case next_cmd:
        of 0x0: discard
        of 0x1: swaps(EXEC) # TODO might change readers
        of 0x2: later(EXEC)
        of 0x3: merge(EXEC)
        # of 0x4: sifts(EXEC) # TODO might change readers
        of 0x5: execs(EXEC, STACK)
        of 0x6: delev(EXEC)
        # of 0x7: equal(EXEC)
        of 0x8: halve(EXEC)
        # of 0x9: uplev(EXEC)
        of 0xA: reads(EXEC)
        # of 0xB: dealc(EXEC) # TODO might change readers
        of 0xC: split(EXEC) # TODO might change readers
        # of 0xD: polar(EXEC)
        of 0xE: doalc(EXEC) # TODO might change readers
        # of 0xF: input(EXEC) # TODO might change readers
        else: discard
      next_cmd = EXEC.exec.moveThenGet(debug=debug)
    when debug: echo &"Exited with next_cmd {next_cmd}"
    # WARN
    # Coming out here means that the EXEC has terminated.
    # We should be careful about ... memory leaks.
    if STACK.len == 0:
      when debug: echo "Stack length zero, pop"
      return
    EXEC = STACK.pop()

# TODO   :)
# We can make the highest EXEC take in input from the stdin or something.
# proc repl()

# TODO list
# node -> reader map and refpassing contained within the run/interpret context
# swaps fix
# split fix
# doalc fix
# dealc
# helloworld2 example confirmed
# input
# uplev
# cat confirmed
# polar
# equal
# truth machine confirmed
# Clean and read files
# Bin/Hex viewing of path (complete with shorthand for pure nodes larger than a group of 8)
# sifts

when isMainModule:

  const prg = "$$$(([]!)/([])):((/[])/([]!/[]!)):(/[])::[/([]!/[])]!:[[[]]!]:[([]!)/[/[]!]!]:[/([]!/[])]!:[([]!)/(/[])]:((/[])/[]):(/([]!)):([[]]!/[[]!]!):[[[]/[]]]!:"
  var hexData: seq[uint8]
  for c in prg:
    hexData.add c.symToVal

  # Initialize program stack and base executor
  var STACK: seq[Executor]
  var EXEC = hexData.initTLP

  # echo EXEC.exec.path.dataRoot

  # execs(EXEC, STACK)

  # prg.interpret(EXEC)

  run(EXEC)
