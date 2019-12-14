import deques
import dao_errors

const
  DEBUG = true

type
  DaoNodeKind* = enum
    DnkNEG, DnkPOS             ##  Pure nodes of greater than 8 bits
    DnkMIX                     ##  Node of greater than 8 bits with heterogeneous children
    Dnk8                       ##  Mixed node of 8 or fewer bits.

  DaoNode* = ref object
    parent*: DaoNode
    pow*: uint64               ##  2^pow bits "contained" by this node.
    case kind*: DaoNodeKind
      of DnkMIX:
        lc*, rc*: DaoNode
      of Dnk8:
        val*: uint8
      of DnkNEG, DnkPOS: discard

  ReaderMode* = enum
    RmPOS                      ##  On interior of uint8 node (8/4/2) or uniform node (POS/NEG)
    RmNODE                     ##  On exterior, at node level

  Reader* = object
    ## The reader allows for traversal of DaoNodes.
    path*:        Path         ##  The path operated on by the Reader
    node*:        DaoNode      ##  Location of data node
    mode*:        ReaderMode   ##  If in the interior of a complex node
    pow*:         uint64       ##  record pow2
    idx*:         uint64       ##  and index from left at that resolution

  StoredReader* = object
    ## A reader stored on a path free from nodes
    path*:        Path         ##  Path operating on
    rootPow*:     uint64       ##  Pow of the path's dataRoot at time of storage
    pow*:         uint64       ##  Pow of the reader at time of storage
    moves*:       seq[uint64]  ##  Left/Right instructions needed to restore the reader
    nBitsInFirstMove*: uint64  ##  Number of bits used in the last move in moves to actually encode moves

  Executor* = object
    ## The executor reads instructions from a tracked location in the exec node
    ##     and operates on the data node.
    oplevel*:     uint8        ##  Operating level
    execStart*:   Reader       ##  Location where execution began (return to when UPLEV)
    exec*:        Reader       ##  Tracker for execution
    data*:        Reader       ##  Tracker for target of execution
    wasMoved*:    bool         ##  If the executor was moved (thus far, by uplev only) and shouldn't increment for a turn

  StoredExecutor* = object
    oplevel*:     uint8
    execStart*:   StoredReader
    exec*:        StoredReader
    data*:        StoredReader

  StoredReaderRef* = ref StoredReader

  Path* = ref object
    owner*:       Path         ##  Parent program (may be nil)
    child*:       Path         ##  Child program (may be nil)
    depth*:       uint64       ##  How deep is this in the program tree?
    dataRoot*:    DaoNode      ##  Points to the root of the data owned by this path


proc treeify*(bytes: openarray[int8 | uint8]): tuple[firstHex: DaoNode, root: DaoNode]=
  if bytes.len == 0: return

  var nodes = initDeque[DaoNode]()

  for i in countup(0, bytes.len - 2, 2):
    let val = (bytes[i] shl 4) or bytes[i+1]
    nodes.addLast(DaoNode(kind: Dnk8, pow: 3, val: val, parent: nil))

  if bytes.len mod 2 == 1:
    # Take care of remainder if it exists.
    nodes.addLast DaoNode(kind: Dnk8, pow: 3, val: bytes[^1] shl 4)

  result.firstHex = nodes[0]

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
        # Blank dnk8 - escalate into DnkNEG
        nodes.addLast DaoNode(kind: DnkNEG, pow: lc.pow + 1)
      else:
        # Otherwise, replace as lc of new mix parent with empty rc
        let parent = DaoNode(kind: DnkMIX, pow: lc.pow + 1, lc: lc)
        lc.parent = parent
        if parent.pow > 3'u64:
          parent.rc = DaoNode(kind: DnkNEG, parent: parent, pow: lc.pow)
        elif parent.pow == 3'u64:
          parent.rc = DaoNode(kind: Dnk8, parent: parent, pow: 3, val: 0'u8)
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

  result.root = nodes.popFirst()

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

  proc `$`*(path: Path): string =
    result.add fmt"[{path.depth: 3}"
    result.add if path.owner != nil: '^' else: ' '
    result.add if path.child != nil: 'v' else: ' '
    result.add ']'

  proc `$`*(node: DaoNode): string =
    if node == nil:
      return "(nil)"
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

  proc readNode*(reader: Reader): string =
    let node = reader.node
    if node == nil:
      return "(nil)"
    case node.kind:
      of DnkMIX:
        result.add(fmt"({node.lc} {node.rc})")
        assert node.lc.parent == node
        assert node.rc.parent == node
      of Dnk8:
        var val: string
        if reader.pow == 0:   val = "b" & "01"[node.val shr (7 - (reader.idx and 0b111'u8).int) and 1'u8]
        elif reader.pow == 1: val = "b" & (node.val shr ((3 - (reader.idx and 0b11'u8).int) shl 1) and 0b11'u8).BiggestInt.toBin(2)
        elif reader.pow == 2: val = "x" & (node.val shr ((1 - (reader.idx and 0b1'u8).int) shl 2) and 0xF'u8).toHex[1]
        elif reader.pow == 3: val = "x" & node.val.toHex
        else: Unreachable("READNODE: Impossible Dnk8 pow.")

        result.add(fmt"({val})")

      of DnkNEG: result.add(fmt"({node.pow} 0)")
      of DnkPOS: result.add(fmt"({node.pow} 1)")

  proc `$`*(reader: Reader): string =
    let nodeStr = if reader.node != nil: fmt"{$reader.node.kind} {reader.readNode}" else: "nil"
    case reader.mode:
      of RmNODE: fmt"(Path: {reader.path} pow: {reader.pow} idx: {reader.idx} Node: {nodeStr})"
      of RmPOS:  fmt"(Path: {reader.path} pow: {reader.pow} idx: {reader.idx} NPos: {nodeStr})"

  proc `==`*(a, b: Reader): bool =
    (a.path == b.path) and (a.pow == b.pow) and (a.idx == b.idx) and (a.node == b.node) and (a.mode == b.mode)
