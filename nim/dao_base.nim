import deques
import dao_errors

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

  result.root = nodes.popFirst()