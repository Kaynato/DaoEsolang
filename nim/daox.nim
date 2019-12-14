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

import os
import parseopt

import strutils
import strformat

import dao_base
import dao_errors
import reader

const
  OP_NAMES = [
    "IDLES", "SWAPS", "LATER", "MERGE",
    "SIFTS", "EXECS", "DELEV", "EQUAL",
    "HALVE", "UPLEV", "READS", "DEALC",
    "SPLIT", "POLAR", "DOALC", "INPUT"
  ]

  OP_SYM = ".!/)%#>=(<:S[*$;"
  FILE_SYMBOLIC = ".dao"
  FILE_COMPILED = ".wuwei"

  VERSION_STRING = """
                                     
		                   =====             
		              =====#####=====        
		           ===###############===     
		         ===###################===   
		        ==#######################==  
		       ==#####   ########====#####== 
		      ==#####     #####==    ===###==
		      ==#####     ####=         ==##=
		      = =#####   ####=    ###     =#=
		      =  ==#########=    #####     ==
		      ==   ===####==     #####     ==
		       ==     ====        ###     == 
		        ==                       ==  
		         ===                   ===   
		           ===               ===     
		              =====     =====        
		                   =====             
		                                     
		        ===========================  
		               D  A  O  Y  U         
		        ===========================  
		                                     
		[[DaoLanguage (Daoyu) Interpreter - Nim ver.]]
		              Kaynato 2019
		              Version 0.6.0
"""


## Utility
proc store*(e: Executor): StoredExecutor =
  result.oplevel = e.oplevel
  result.data = e.data.store
  result.exec = e.exec.store
  result.execStart = e.execStart.store

proc retrieve*(se: StoredExecutor): Executor =
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
  withSafeModify(EXEC):
    swapsReader(EXEC.data)

proc later(EXEC: var Executor) =
  if EXEC.oplevel < 4'u8:
    ## Ordinary LATER
    discard laterReader(EXEC.data)
  else:
    # TODO potentially undefined behavior when lines at end
    discard linesReader(EXEC.data)

proc merge(EXEC: var Executor) =
  if EXEC.oplevel > 6'u8: return
  ## Ascend EXEC.data
  discard mergeReader(EXEC.data)

proc sifts(EXEC: var Executor) =
  Todo("Implement SIFTS")

proc execs(EXEC: var Executor, STACK: var seq[StoredExecutor]) =
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
  STACK.add(EXEC.store)

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

proc delev(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8:
    dec EXEC.oplevel

proc equal(EXEC: var Executor, debug: int = 0) =
  if EXEC.oplevel > 5'u8: return
  ## Execute next cmd only if equal, else skip
  # Nop if equal
  if leftmostBit(EXEC.data) == rightmostBit(EXEC.data): return
  # Else, skip
  discard linesReader(EXEC.exec)
  if debug > 0: echo fmt" = skip {OP_SYM[EXEC.exec.getHex()]} :"

proc halve(EXEC: var Executor) =
  ## Descend into node left half.
  if EXEC.oplevel > 6'u8: return
  halveReader(EXEC.data)

proc uplev(EXEC: var Executor) =
  if EXEC.oplevel > 8'u8: return
  EXEC.oplevel += 1
  # This copies, right? Right. Pretty sure.
  EXEC.exec = EXEC.execStart
  EXEC.wasMoved = true

proc reads(EXEC: var Executor) =
  if EXEC.oplevel > 5'u8: return
  readsReader(EXEC.data)

proc dealc(EXEC: var Executor) =
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

proc split(EXEC: var Executor) =
  if EXEC.oplevel > 6'u8: return
  if EXEC.oplevel == 0'u8:
    withSafeModify(EXEC):
      splitReader(EXEC.data)
  halveReader(EXEC.data)

proc polar(EXEC: var Executor, debug: int = 0) =
  if EXEC.oplevel > 3'u8: return
  ## Execute next cmd only if polar, else skip
  # Nop if polar
  if leftmostBit(EXEC.data) > rightmostBit(EXEC.data): return
  # Else, skip
  discard linesReader(EXEC.exec)
  if debug > 0: echo fmt" * skip {OP_SYM[EXEC.exec.getHex()]} :"

proc doalc(EXEC: var Executor) =
  if EXEC.oplevel > 0'u8: return
  # Allocation is performed on the root of the reader's node, not the executing path.
  # Create parent
  withSafeModify(EXEC):
    doalcReader(EXEC.data)

proc input(EXEC: var Executor, inputStream: File) =
  if EXEC.oplevel > 5'u8: return
  withSafeModify(EXEC):
    inputReader(EXEC.data, inputStream)

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
    else: 0xFF'u8

proc hexToVal(ch: char): uint8 =
  case ch:
    of '0'..'9': ch.uint8 - '0'.uint8
    of 'A'..'F': ch.uint8 - 'A'.uint8 + 0xA'u8
    else: 0xFF'u8

proc initTLP(bytes: seq[uint8]): Executor =
  ## Convert seq of hexes into an equivalent Path
  let tree = treeify(bytes)
  # Now we have made the tree for the TLP.
  var tlp = initPath(dataRoot = tree.root)
  # Special case child construction
  var child = initPath(parent = tlp)
  # Special case executor construction
  result.oplevel = 0
  result.execStart = Reader(path: tlp, mode: RmPOS, node: tree.firstHex, pow: 2, idx: 0)
  result.exec = Reader(path: tlp, mode: RmPOS, node: tree.firstHex, pow: 2, idx: 0)
  result.data = Reader(path: child, mode: RmPOS, node: child.dataRoot, pow: 0, idx: 0)

proc run(EXEC: var Executor, inputStream: File, debug: int = 0) =
  ## Run until termination
  var STACK: seq[StoredExecutor]

  if debug > 0: echo fmt"Init lv {EXEC.oplevel} : data : {EXEC.data} "
  if debug > 1: echo fmt"          : path : {EXEC.data.path.dataRoot}"

  var next_cmd: uint8 = EXEC.exec.getHex()
  while true:

    while next_cmd != 0xFF:
      case next_cmd:
        of 0x0: discard
        of 0x1: swaps(EXEC)
        of 0x2: later(EXEC)
        of 0x3: merge(EXEC)
        of 0x4: sifts(EXEC)
        of 0x5: execs(EXEC, STACK)
        of 0x6: delev(EXEC)
        of 0x7: equal(EXEC, debug)
        of 0x8: halve(EXEC)
        of 0x9: uplev(EXEC)
        of 0xA: reads(EXEC)
        of 0xB: dealc(EXEC)
        of 0xC: split(EXEC)
        of 0xD: polar(EXEC, debug)
        of 0xE: doalc(EXEC)
        of 0xF: input(EXEC, inputStream)
        else: discard

      if debug > 0: echo fmt"op {OP_SYM[next_cmd]} lv {EXEC.oplevel} : data : {EXEC.data} "
      if debug > 1: echo fmt"          : path : {EXEC.data.path.dataRoot}"
      if EXEC.exec.pow != 2:       Unreachable("moveThenGet: Exec reader has non-2 pow")
      if EXEC.exec.mode == RmNODE: Unreachable("moveThenGet: Exec reader was pow 2 but was reading Nodes")

      # If data path destroyed, stop execution
      if EXEC.data.path == nil or EXEC.data.path.dataRoot == nil:
        if debug > 0: echo "Nil data path, break"
        break
      if EXEC.exec.path == nil or EXEC.exec.path.dataRoot == nil:
        if debug > 0: echo "Nil exec path, break"
        break

      if debug > 0:
        var stored = EXEC.data.store
        if EXEC.data != stored.retrieve:
          echo "WARN: Store and retrieve CHANGED for readers!"
          echo "Original:\n\t", EXEC.data
          echo "Stored:\n\t", stored
          echo "Retrieved:\n\t", stored.retrieve
          assert false

      # Determine next command
      next_cmd = 
        if EXEC.wasMoved: EXEC.wasMoved = false; EXEC.exec.getHex()
        elif linesReader(EXEC.exec):             EXEC.exec.getHex()
        else:                                    0xFF

    if debug > 0: echo &"Exited with next_cmd {next_cmd}"
    # WARN
    # Coming out here means that the EXEC has terminated.
    # We should be careful about ... memory leaks.
    if STACK.len == 0:
      if debug > 0: echo "Stack length zero, terminating execution"
      return
    EXEC = STACK.pop.retrieve

proc printHelp() =
  echo "Argument syntax: [main args] [command] [command args]"
  echo ""
  echo "Main args:"

  echo "    "
  echo "    (--version / -v)"
  echo "        Prints the version message."

  # TODO  Bin/Hex viewing of path (complete with shorthand for pure nodes larger than a group of 8)
  # TODO  Print path data as daoyu program in daoyu symbols
  # sifts 
  echo "    "
  echo "    (--debug / -d)=<value>"
  echo "        Set the debug value."
  echo "            0: Same as not setting debug."
  echo "            1: Print the program tree and current reader state every step."
  echo "            2: Print the above and also the entire selected path every step."

  echo ""
  echo "Commands:"

  echo "    "
  echo "    (help)"
  echo "        Prints the helpfile message."

  echo "    "
  echo "    (interpret / run / r) (--mode / -m) <auto/sym/hex/raw> <filename>"
  echo "        Interpret the file as a Daoyu program."
  echo "        Use -m to set interpretation mode."
  echo "        "
  echo "        `auto` (default)"
  echo "            By default, -m is `auto` and infers the mode of the file from its file extension. "
  echo "            .dao   -> SYM interpretation"
  echo "            .wuwei -> RAW interpretation"
  echo "        "
  echo "        `sym`"
  echo "            In `sym` interpretation mode, the Daoyu symbols are read from file."
  echo "            Invalid characters will be ignored."
  echo "            Characters in a line after `@` will be treated as comments and ignored."
  echo "        "
  echo "        `hex`"
  echo "            In `hex` interpretation mode, the string representations of hexadecimal characters will be read from file."
  echo "            e.g. 0123456789ABCDEF"
  echo "            Invalid characters will be ignored."
  echo "            Characters in a line after `@` will be treated as comments and ignored."
  echo "        "
  echo "        `raw`"
  echo "            In `raw` interpretation mode, the raw bytes will be read."

  # TODO ver 0.8.0
  # echo ""
  # echo "(listen) (--mode / -m) <auto/sym/hex/raw> <port>"
  # echo "\tListen to the port for daoyu programs."

  # TODO ver 0.9.0
  # echo ""
  # echo "(repl / interact) (--mode / -m) <auto/sym/hex/raw>"
  # echo "\tBegin an interactive REPL daoyu environment."
  # echo "\tInputs will be dynamically added to a top level program."
  # echo "\tBehavior may differ from standard interpretation."

  # echo ""
  # echo "\t"
  # echo "\t\t"



type
  ExecMode = enum
    ExecModeAuto, ExecModeSym, ExecModeHex, ExecModeByte
  DispMode = enum
    DispModeBinHex, DispModeSym

proc readAllFileBytes(filename: string): seq[uint8] =
  let file = open(filename, mode=FileMode.fmRead)
  result.setLen(file.getFileSize()-1)
  discard file.readBytes(result, 0, result.len-1)

proc cleanSymFile(filename: string): seq[uint8] =
  ## Clean an input symbol file
  for line in lines(filename):
    for c in line:
      if c == '@': break   # Line comment
      let val = c.symToVal # Add value if non-ignored value
      if val != 0xFF'u8:
        result.add val

proc cleanHexFile(filename: string): seq[uint8] =
  ## Clean an input literal hex file
  for line in lines(filename):
    for c in line:
      case c:
        of '@': break # Line comment
        of '0'..'9': result.add (c.uint8 - '0'.uint8) # Symbol
        of 'A'..'F': result.add (c.uint8 - 'A'.uint8 + 0xA'u8)
        else: continue # Ignore

proc parseRun(parser: var OptParser, debugMode: int) =
  var targets: seq[string]
  var modeWasSet = false
  var execMode = ExecModeAuto
  while true:
    parser.next()
    case parser.kind:
      of cmdEnd: break
      of cmdShortOption, cmdLongOption:
        # Args passed to run / interpret
        # I mean, there shouldn't be many for now. We might see differences later.
        case parser.key:
          of "m", "mode", "execmode":
            if not modeWasSet:
              execMode =
                case parser.val:
                of "auto", "": ExecModeAuto
                of "sym": ExecModeSym
                of "hex": ExecModeHex
                of "byte", "data", "raw": ExecModeByte
                else:
                  echo &"Unrecognized execution mode \"{parser.val}\" will be ignored."
                  ExecModeAuto
              modeWasSet = true
            else:
              echo "Warning: Attempted to set mode more than once. Mode was already set to ", $execMode
          else:
            echo "Unrecognized flag {parser.key}"
      of cmdArgument:
        # Run the first thing we see, meow.
        targets.add parser.key
  if targets.len > 1:
    echo "More than one argument was passed in. Only one argument (the file to run) is expected."
  else:
    let target = targets[0]

    if not fileExists(target):
      echo &"Error: Could not find file {target}"
      return

    let hexdata: seq[uint8] = 
      case execMode:
      of ExecModeAuto:
        if target.endsWith(FILE_COMPILED):   readAllFileBytes(target)
        elif target.endsWith(FILE_SYMBOLIC): cleanSymFile(target)
        else:
          echo "Unknown filetype, reading as bytestream"
          readAllFileBytes(target)
      of ExecModeSym:  cleanSymFile(target)
      of ExecModeHex:  cleanHexFile(target)
      of ExecModeByte: readAllFileBytes(target)

    # Initialize program stack and base executor
    var EXEC = hexData.initTLP

    if debugMode > 0: echo EXEC.exec.path.dataRoot

    EXEC.run(stdin, debug=debugMode)


when isMainModule:
  # Hmm... dealc can be used to kick the instruction pointer backwards
  var parser = initOptParser()
  var debugMode: int

  while true:
    parser.next()
    case parser.kind:
      of cmdArgument:
        # Sets program intention
        case parser.key:
        of "c", "compile":
          Todo("Compile")
        of "r", "interpret", "run":
          parseRun(parser, debugMode)
          break
        of "help":
          printHelp()
          break
        else:
          echo "Unknown argument: ", parser.key
          break
      of cmdShortOption, cmdLongOption:
        # TODO args passed to daox program directly
        case parser.key:
        of "d", "debug":
          debugMode = parser.val.parseInt
        of "v", "version":
          echo VERSION_STRING
          break
        else:
          if parser.val == "": echo &"Unknown option: {parser.key}"
          else:                echo &"Unknown option and value: {parser.key} {parser.val}"
      of cmdEnd:
        echo VERSION_STRING
        printHelp()
        break

  # const prg = "$$$(/(/("
  # const prg = "$$$(([]!)/([])):((/[])/([]!/[]!)):(/[])::[/([]!/[])]!:[[[]]!]:[([]!)/[/[]!]!]:[/([]!/[])]!:[([]!)/(/[])]:((/[])/[]):(/([]!)):([[]]!/[[]!]!):[[[]/[]]]!:"
  
  # Helloworld 2
  # const prg = "))))))))/:((((((S...............%(>#>[>[>;/.==>;=/>[>%/!.:......"

  # Cat
  # const prg = "$$$>;:<"

  # Uplev test
  # const prg = "$..<"

  # Truth machine
  # const prg = "$$$;(*[=[*]*S=S=S=S=S(!)=S*S=S=S(/!)=!*S*S=S=S)!=[=]*S=S=S=S((!))=[=]*S=S=S=S(!)=[=]*S=S=S=S(!)!$$$$((/(/(/(/([]!/[])))))/(((([]!/[]!)/([]!/[]!))/(([/[]!]!/([]/[]))/([/[]!]!/[/[]]!)))/((([/[]!]!/[/[]!])/(([]/[])/([])))/((([])/([]))/(([])/([]/[]!))))))((/(/(/(/#))))SSSSSSS"

  