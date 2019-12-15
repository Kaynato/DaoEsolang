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
import dao_reader
import dao_impl

const
  OP_NAMES = [
    "IDLES", "SWAPS", "LATER", "MERGE",
    "SIFTS", "EXECS", "DELEV", "EQUAL",
    "HALVE", "UPLEV", "READS", "DEALC",
    "SPLIT", "POLAR", "DOALC", "INPUT"
  ]
  
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

## Operation implementation
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

proc initTLP(tlpData: TLPData): Executor =
  ## Convert seq of hexes into an equivalent Path

  var tlp = initPath(dataRoot = tlpData.root)
  # Special case child construction
  var child = initPath(parent = tlp)
  # Special case executor construction
  result.oplevel = 0
  result.execStart = Reader(path: tlp, mode: RmPOS, node: tlpData.firstHex, pow: 2, idx: 0)
  result.exec = Reader(path: tlp, mode: RmPOS, node: tlpData.firstHex, pow: 2, idx: 0)
  result.data = Reader(path: child, mode: RmPOS, node: child.dataRoot, pow: 0, idx: 0)

proc run(EXEC: var Executor, inputStream: File, debug: int = 0) =
  ## Run until termination
  if debug > 0: echo fmt"Init lv {EXEC.oplevel} : data : {EXEC.data} "
  if debug > 1: echo fmt"          : path : {EXEC.data.path.dataRoot}"

  var STACK: ExecutorStack
  var next_cmd: range[0'i8..16'i8] = EXEC.exec.getHex()
  while true:

    while next_cmd != 16'i8:
      {.computedGoto.}
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
        of 0x10: discard

      if debug > 0: echo fmt"op {LOOKUP_OP_SYM[next_cmd]} lv {EXEC.oplevel} : data : {EXEC.data} "
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

      if debug > 0: EXEC.data.checkReaderStores()

      # Determine next command
      if EXEC.wasMoved:
        EXEC.wasMoved = false
        next_cmd = EXEC.exec.getHex()
      elif linesReader(EXEC.exec):
        next_cmd = EXEC.exec.getHex()
      else:
        break

    if debug > 0: echo &"Exited with next_cmd {next_cmd}"
    # WARN
    # Coming out here means that the EXEC has terminated.
    # We should be careful about ... memory leaks.
    if STACK.len == 0:
      if debug > 0: echo "Stack length zero, terminating execution"
      return
    EXEC = STACK.retrieve

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

proc readAllFileBytes(filename: string): TLPData =
  let file = open(filename, mode=FileMode.fmRead)
  let bytes = file.getFileSize()
  var data: seq[uint8]
  data.setLen(bytes)
  discard file.readBytes(data, 0, bytes)
  return data.treeify(as_hex=false)

proc cleanSymFile(filename: string): TLPData =
  ## Clean an input symbol file
  var hexes: seq[uint8]
  for line in lines(filename):
    for c in line:
      if c == '@': break   # Line comment
      let val = c.symToVal # Add value if non-ignored value
      if val != 0xFF'u8:
        hexes.add val
  return hexes.treeify(as_hex=true)

proc cleanHexFile(filename: string): TLPData =
  ## Clean an input literal hex file
  var hexes: seq[uint8]
  for line in lines(filename):
    for c in line:
      case c:
        of '@': break # Line comment
        of '0'..'9': hexes.add (c.uint8 - '0'.uint8) # Symbol
        of 'A'..'F': hexes.add (c.uint8 - 'A'.uint8 + 0xA'u8)
        else: continue # Ignore
  return hexes.treeify(as_hex=true)

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
        break
  if targets.len > 1:
    echo "More than one argument was passed in. Only one argument (the file to run) is expected."
  elif targets.len < 1:
    echo "No file specified."
  else:
    let target = targets[0]

    if not fileExists(target):
      echo &"Error: Could not find file {target}"
      return

    let tlpData: TLPData = 
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
    var EXEC = tlpData.initTLP

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
