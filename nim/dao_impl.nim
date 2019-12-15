#[ 
  Implements daoyu functions.
]#

import dao_base
import dao_errors
import dao_reader

## Utility
proc store(e: Executor): StoredExecutor =
  result.oplevel = e.oplevel
  result.data = e.data.store
  result.exec = e.exec.store
  result.execStart = e.execStart.store

proc retrieve(se: StoredExecutor): Executor =
  result.oplevel = se.oplevel
  result.data = se.data.retrieve
  result.exec = se.exec.retrieve
  result.execStart = se.execStart.retrieve
  result.wasMoved = false

template withSafeModify(e: var Executor, body: untyped): untyped =
  var sreader: StoredReaderRef = nil

  if e.data.path == e.exec.path:
    sreader.new
    sreader[] = e.exec.store

  body

  if sreader != nil:
    e.exec = sreader[].retrieve

proc checkReaderStores*(reader: Reader) =
  var stored = reader.store
  if reader != stored.retrieve:
    echo "WARN: Store and retrieve CHANGED for readers! It's likely that the program moved into an illegal state."
    echo "Original:\n\t", reader
    echo "Stored:\n\t", stored
    echo "Retrieved:\n\t", stored.retrieve
    Unreachable("READER STATE INCONSISTENT")

proc store*(stack: var ExecutorStack, executor: Executor) =
  stack.add(executor.store)

proc retrieve*(stack: var ExecutorStack): Executor =
  stack.pop().retrieve

proc swaps*(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8: return
  withSafeModify(EXEC):
    swapsReader(EXEC.data)

proc later*(EXEC: var Executor) =
  if EXEC.oplevel < 4'u8:
    ## Ordinary LATER
    discard laterReader(EXEC.data)
  else:
    # TODO potentially undefined behavior when lines at end
    discard linesReader(EXEC.data)

proc merge*(EXEC: var Executor) =
  if EXEC.oplevel > 6'u8: return
  ## Ascend EXEC.data
  discard mergeReader(EXEC.data)

proc sifts*(EXEC: var Executor) =
  # Todo("SIFTS is deprecated.")
  discard

proc execs*(EXEC: var Executor, STACK: var seq[StoredExecutor]) =
  if EXEC.oplevel > 7'u8: return

  # Exit if the exec reader would be crushed
  if EXEC.data.path.dataRoot.pow < 2'u64: return

  # Copy the reader and navigate to the leftmost pow3 position in selection
  var reader = EXEC.data

  case reader.mode:
  of RmPOS:
    while reader.pow < 2'u64:
      discard mergeReader(reader)
    if reader.pow == reader.node.pow:
      reader.mode = RmNODE
  of RmNODE:
    if reader.node.pow > 2'u64:
      while reader.node.pow > 2'u64:
        halveReader(reader)
    elif reader.pow == 2'u64:
      discard
    else:
      Unreachable("EXECS: Reader pow was < 2 despite being RmNODE")

  # Store the old reader
  STACK.store(EXEC)

  # If child of target path is nil, create a new child
  let child = if EXEC.data.path.child == nil:
    initPath(parent = EXEC.data.path)
  else:
    EXEC.data.path.child

  # Create a new executor, cloning the current reader for its position
  let mode = if child.dataRoot.pow >= 3: RmNODE else: RmPOS
  let dataReader = Reader(path: child, node: child.dataRoot, mode: mode, pow: child.dataRoot.pow, idx: 0)
  var newExec = Executor(oplevel: 0, execStart: reader, exec: reader, data: dataReader, wasMoved: true)

  # Swap out the executor
  EXEC = newExec

proc delev*(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8:
    dec EXEC.oplevel

proc equal*(EXEC: var Executor, debug: int = 0) =
  if EXEC.oplevel > 5'u8: return
  ## Execute next cmd only if equal, else skip
  # Nop if equal
  if leftmostBit(EXEC.data) == rightmostBit(EXEC.data): return
  # Else, skip
  discard linesReader(EXEC.exec)
  if debug > 0: echo " = skip ", LOOKUP_OP_SYM[EXEC.exec.getHex()], " :"

proc halve*(EXEC: var Executor) =
  ## Descend into node left half.
  if EXEC.oplevel > 6'u8: return
  halveReader(EXEC.data)

proc uplev*(EXEC: var Executor) =
  if EXEC.oplevel > 8'u8: return
  EXEC.oplevel += 1
  # This copies, right? Right. Pretty sure.
  EXEC.exec = EXEC.execStart
  EXEC.wasMoved = true

proc reads*(EXEC: var Executor) =
  if EXEC.oplevel > 5'u8: return
  readsReader(EXEC.data)

proc dealc*(EXEC: var Executor) =
  if EXEC.oplevel > 2'u8: return
  
  let oldpow = EXEC.data.pow
  withSafeModify(EXEC):
    dealcReader(EXEC.data)

  let newpow = EXEC.data.pow
  if newpow == oldpow - 1:
    discard # Implicitly halved already
  elif newpow == oldpow:
    halveReader(EXEC.data) # Have to explicitly halve
  else:
    Unreachable("DEALC: Data pow changed beyond implicit halving.")

proc split*(EXEC: var Executor) =
  if EXEC.oplevel > 6'u8: return
  if EXEC.oplevel == 0'u8:
    withSafeModify(EXEC):
      splitReader(EXEC.data)
  halveReader(EXEC.data)

proc polar*(EXEC: var Executor, debug: int = 0) =
  if EXEC.oplevel > 3'u8: return
  ## Execute next cmd only if polar, else skip
  # Nop if polar
  if leftmostBit(EXEC.data) > rightmostBit(EXEC.data): return
  # Else, skip
  discard linesReader(EXEC.exec)
  if debug > 0: echo " * skip ", LOOKUP_OP_SYM[EXEC.exec.getHex()], " :"

proc doalc*(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8: return
  # Allocation is performed on the root of the reader's node, not the executing path.
  # Create parent
  withSafeModify(EXEC):
    doalcReader(EXEC.data)

proc input*(EXEC: var Executor, inputStream: File) =
  if EXEC.oplevel > 5'u8: return
  withSafeModify(EXEC):
    inputReader(EXEC.data, inputStream)