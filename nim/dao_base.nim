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
    RmNODE                     ##  On exterior, at node level
    RmPOS                      ##  On interior of uint8 node (8/4/2) or uniform node (POS/NEG)

  Reader* = object
    ## The reader allows for traversal of DaoNodes.
    path*:        Path         ##  The path operated on by the Reader
    node*:        DaoNode      ##  Location of data node
    mode*:        ReaderMode   ##  If in the interior of a complex node
    pow*:         uint64       ##  record pow2
    idx*:         uint64       ##  and index from left at that resolution

  Executor* = object
    ## The executor reads instructions from a tracked location in the exec node
    ##     and operates on the data node.
    oplevel*:     uint8        ##  Operating level
    execStart*:   Reader       ##  Location where execution began (return to when UPLEV)
    exec*:        Reader       ##  Tracker for execution
    data*:        Reader       ##  Tracker for target of execution

  Path* = ref object
    owner*:       Path         ##  Parent program (may be nil)
    child*:       Path         ##  Child program (may be nil)
    depth*:       uint64       ##  How deep is this in the program tree?
    dataRoot*:    DaoNode      ##  Points to the root of the data owned by this path