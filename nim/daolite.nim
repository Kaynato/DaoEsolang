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
##  DaoLanguage / Daoyu Compiler and Interpreter.
##  Kaynato - 2016
##  See splash() for details.
## 

import strutils
import bitops
import os

const
  BITS_IN_BYTE* = 8
  BITS_IN_CELL* = sizeof(uint64) * 8
  INPUT_DELIMITER* = '@'

type
  Path* = ref object
    owner*: ptr PATH            ##  OWNER      PROGRAM
    child*: ptr PATH            ##  CHILD      PROGRAM
    prg_data*: ptr uint64       ## 		   DATA
    prg_allocbits*: uint64     ##  OPEN	   DATA   BITS
    prg_index*: uint64         ##  INSTRUCTION POINTER
    prg_level*: uint8         ##  OPERATING   LEVEL
    sel_length*: uint64        ##  LENGTH OF SELECTION
    sel_index*: uint64         ##  INDEX  OF SELECTION
    prg_floor*: cuint          ##  FLOOR  OF PATH
    prg_start*: uint64         ##  START  OF RUNNING
  
proc interpret*(a2: string)
proc swaps*(a2: Path)
proc later*(a2: Path)
proc merge*(a2: Path)
proc sifts*(a2: Path)
proc delev*(a2: Path)
proc equal*(a2: Path)
proc halve*(a2: Path)
proc uplev*(a2: Path)
proc reads*(a2: Path)
proc dealc*(a2: Path)
proc split*(a2: Path)
proc polar*(a2: Path)
proc doalc*(a2: Path)
proc input*(a2: Path)
proc execs*(a2: Path; a3: Path)
proc getInput*(): uint8
proc algn*(a2: Path): uint8
proc getChar*(a2: uint8): char
proc bin*(a2: uint64): cstring
proc str_dup*(s: cstring): cstring
proc l_to_str*(a2: uint64; a3: uint8; a4: uint8): cstring
proc skip*()
proc write_by_bit_index*(a2: Path; a3: uint64; a4: uint64; a5: uint64)
proc getNybble*(ch: char): uint8
proc read_by_bit_index*(a2: Path; a3: uint64; a4: uint64): uint64
proc mask*(a2: cint): uint64
proc ptwo_round*(a2: int): uint64

var command*: uint8 = 0.uint8
var doloop*: cint = 1

proc initPath(): Path =
  Path(owner: nil, child: nil, prg_data: nil,
    prg_allocbits: 1,
    prg_index: 0,
    prg_level: 0,
    sel_length: 1,
    sel_index: 0,
    prg_floor: 0,
    prg_start: 0)

var
  P_RUNNING*: Path = nil
  P_WRITTEN*: Path = nil

var symbols*: cstring = ".!/)%#>=(<:S[*$;"

var inputptr*: cstring = nil

proc getNybble*(ch: char): uint8 =
  case ch:
    of '.': return 0x00000000
    of '!': return 0x00000001
    of '/': return 0x00000002
    of ']', ')': return 0x00000003
    of '%': return 0x00000004
    of '#': return 0x00000005
    of '>': return 0x00000006
    of '=': return 0x00000007
    of '(': return 0x00000008
    of '<': return 0x00000009
    of ':': return 0x0000000A
    of 'S': return 0x0000000B
    of '[': return 0x0000000C
    of '*': return 0x0000000D
    of '$': return 0x0000000E
    of ';': return 0x0000000F
    else: return 0x00000000

proc interpret*(input: string) =
  if input.len == 0:
    return

  ##  Initialize path
  var dao: Path = initPath()
  
  var splitInput = input.split(INPUT_DELIMITER)
  var dao_prg = splitInput[0]

  var dao_input = ""
  if splitInput.len > 1:
    dao_input = splitInput[1]

  ##  Set bit program_length of path
  var program_length = ptwo_round((dao_prg.len + 1) div 2)

  (dao.prg_allocbits) = program_length * 8

  ##  Prevent zero-sized allocation
  if program_length mod sizeof(uint64).uint64 != 0:
    program_length = sizeof(uint64).uint8

  dao.prg_data = cast[ptr uint64](alloc0(program_length))

  program_length = 0
  for c in dao_prg:
    var hex: uint64 = getNybble(c).uint64
    write_by_bit_index(dao, 4 * program_length, 4, hex)
    inc(program_length)


  P_RUNNING = dao
  ##  For the sake of levlim
  ## **************************************************** EXECUTE *****************************************************
  execs(dao, nil)
  dealloc((dao.prg_data))
  (dao.prg_data) = nil
  ## ******************************************************************************************************************
  
proc ptwo_round*(x: int): uint64 =
  ##  Initialize bytes_alloc with the file_size value.
  ##  Shift for first one of file size for rounding
  result = x.uint64
  var shift: uint64 = 0
  ##  Determine leftmost '1' bit of file size.
  while (result shr 1) != 0:
    ##  Shift right until the next shift zeroes it. Track shifts.
    result = result shr 1
    inc(shift)
  result = result shl shift
  if x.uint64 != result: ##  Unshift.
    result = result shl 1

proc getInput*(): uint8 =
  ##  if null return zero
  if isNil inputptr:
    return 0

  if inputptr[] != 0.char: return inc(inputptr)[].uint8

  inputptr = nil
  return 0

proc str_dup*(s: cstring): cstring =
  var d: cstring = malloc(strlen(s) + 1)
  ## Allocate memory
  if d != nil:
    strcpy(d, s)
  return d
  ## Return new memory
  
proc bin*(val: uint64): cstring =
  return l_to_str(val, 32, 2)

proc getChar*(ch: uint8): char =
  if ch > 0x0000000F: return '?'
  return symbols[ch]

proc l_to_str*(val: uint64; len: uint8; radix: uint8): cstring =
  var buf: array[32, char] = ['0']
  var i: cint = 33
  while val and i:
    buf[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUV"[val mod radix]
    dec(i)
    val = val / radix
  while i:
    buf[i] = '0'
    dec(i)
  return addr(buf[2 + (32 - len)])

const
  P_LEN* = (path.sel_length)
  P_IND* = (path.sel_index)
  P_ALC* = (path.prg_allocbits)
  P_LEV* = (path.prg_level)
  P_PIND* = (path.prg_index)
  P_DATA* = (path.prg_data)
  P_OWNER* = (path.owner)
  P_CHILD* = (path.child)
  PR_START* = (P_RUNNING.prg_start)
  PR_LEV* = (P_RUNNING.prg_level)

proc swaps*(path: Path) =
  var i: cuint = 0
  var report: uint64 = 0
  if PR_LEV >= 1: return
  if P_LEN == 1: return
  if P_LEN <= BITS_IN_CELL:
    var half_len: uint64 = P_LEN div 2
    write_by_bit_index(path, P_IND, P_LEN, read_by_bit_index(path, P_IND, half_len) or
        (read_by_bit_index(path, P_IND + half_len, half_len) shl half_len))
    return

  while i < ((P_LEN div BITS_IN_CELL) div 2):
    report = P_DATA[(P_IND div BITS_IN_CELL) + i]
    P_DATA[(P_IND div BITS_IN_CELL) + i] = P_DATA[
        (P_IND div BITS_IN_CELL) + ((P_LEN div BITS_IN_CELL) div 2) + i]
    P_DATA[(P_IND div BITS_IN_CELL) + ((P_LEN div BITS_IN_CELL) div 2) + inc(i)] = report

proc later*(path: Path) =
  if algn(path) or (PR_LEV >= 4): inc(P_IND, P_LEN)
  else: merge(path)
  
proc merge*(path: Path) =
  if PR_LEV >= 7: return
  if P_LEN < P_ALC:
    if not algn(path): dec(P_IND, P_LEN)
    P_LEN = P_LEN shl 1
    return
  if P_OWNER == nil: return
  P_WRITTEN = P_OWNER
  (P_WRITTEN.sel_length) = 1
  (P_WRITTEN.sel_index) = 1

proc sifts*(path: Path) =
  var write: uint64 = P_IND
  var read: uint64 = 0
  if PR_LEV >= 5: return
  while write < P_ALC:
    if write + read < P_ALC:
      while not read_by_bit_index(path, write + read, 4): inc(read, 4)
    if read:
      write_by_bit_index(path, write, 4, if (write + read < P_ALC): read_by_bit_index(
          path, write + read, 4) else: 0)
    inc(write, 4)
    inc(read, 4)

proc execs*(path: Path; caller: Path) =
  var tempNum1: uint64 = 0
  ##  Expedite calculation
  if PR_LEV >= 8:
    return
  P_RUNNING = path
  ##  Set running
  if P_CHILD == nil:
    if (P_CHILD = (alloc0(sizeof(PATH)))) == nil:
      ##  Cover error case
      printf("FATAL ERROR: Unable to allocate memory.")
      return

    memcpy(P_CHILD, initPath(), sizeof(PATH))
    ##  Copy over initialization data
    path.child.owner = path
    ##  Set owner of this new Path
    path.child.prg_floor = (path.prg_floor) + 1
    ##  Set floor of this new Path
    path.child.prg_data = alloc0(sizeof(uint64))
    ##  Set data  of this new Path

  P_WRITTEN = P_CHILD
  ##  Set this as written on
  P_PIND = (P_IND div 4)
  ##  Set program pointer. Rounds down.x
  PR_START = P_PIND
  ##  Track start position
  while doloop and P_PIND < (P_ALC div 4) and path != nil and P_WRITTEN != nil:
    tempNum1 = (P_RUNNING.prg_index)
    command = ((P_RUNNING.prg_data)[(tempNum1 * 4) div BITS_IN_CELL] shr
        (BITS_IN_CELL - ((tempNum1 * 4) mod BITS_IN_CELL) - 4)) and mask(4)
    ##  Calculate command
    case commmand:
      of 0x1: swaps(P_WRITTEN)
      of 0x2: layer(P_WRITTEN)
      of 0x3: merge(P_WRITTEN)
      of 0x4: sifts(P_WRITTEN)
      of 0x5: execs(P_WRITTEN, path)
      of 0x6: delev(P_WRITTEN)
      of 0x7: equal(P_WRITTEN)
      of 0x8: halve(P_WRITTEN)
      of 0x9: uplev(P_WRITTEN)
      of 0xA: reads(P_WRITTEN)
      of 0xB: dealc(P_WRITTEN)
      of 0xC: split(P_WRITTEN)
      of 0xD: polar(P_WRITTEN)
      of 0xE: doalc(P_WRITTEN)
      of 0xF: input(P_WRITTEN)
      else:   discard
    inc(P_PIND)               ##  Execution Loop
  if caller == nil:
    dealloc(P_CHILD)
    P_CHILD = nil
    return

  if not doloop:
    dealloc(P_CHILD)
    P_CHILD = nil
    doloop = 1

  P_RUNNING = caller
  P_WRITTEN = caller.child
  return

proc delev*(path: Path) =
  if PR_LEV > 0: dec(PR_LEV)
  
proc equal*(path: Path) =
  if PR_LEV >= 5: return
  if read_by_bit_index(path, P_IND, 1) xor
      read_by_bit_index(path, P_IND + P_LEN - 1, 1):
    skip()
  
proc halve*(path: Path) =
  if PR_LEV >= 7: return
  if P_LEN > 1:
    P_LEN = P_LEN / 2
    return
  if P_CHILD == nil: return
  P_WRITTEN = P_CHILD
  (P_WRITTEN.sel_length) = (P_WRITTEN.prg_allocbits)

proc uplev*(path: Path) =
  if PR_LEV >= 9: return
  inc(PR_LEV)
  (P_RUNNING.prg_index) = PR_START - 1

proc reads*(path: Path) =
  var pos: clong = P_IND
  if PR_LEV >= 6: return
  if P_LEN < 8:
    var `out`: cstring = bin(read_by_bit_index(path, pos, P_LEN))
    printf("%s", addr(`out`[strlen(`out`) - P_LEN]))
    return

  while pos < (P_IND + P_LEN):
    putchar(read_by_bit_index(path, pos, 8))
    inc(pos, 8)

proc dealc*(path: Path) =
  if PR_LEV >= 2: return
  if P_ALC == 1:
    var report: cint = read_by_bit_index(path, 0, 1)
    if (P_RUNNING.owner) != nil:
      var ownind: uint64 = ((P_RUNNING.owner).prg_index)
      write_by_bit_index(P_RUNNING.owner, (ownind) * 4, 4, report)

    dealloc(P_DATA)
    P_DATA = nil
    doloop = 0
    return

  P_ALC = P_ALC shr 1
  if P_ALC <= 8: realloc(P_DATA, 1)
  else: realloc(P_DATA, P_ALC div 8)
  if P_LEN > 1: halve(path)
  if (P_IND + P_LEN) > P_ALC: dec(P_IND, P_ALC)
  
proc split*(path: Path) =
  if PR_LEV < 1:
    var len: cuint = P_LEN
    if len == 1:
      if P_CHILD == nil: return
      P_WRITTEN = P_CHILD
      (P_WRITTEN.sel_length) = (P_WRITTEN.prg_allocbits)
      split(P_WRITTEN)
      halve(P_WRITTEN)
      return

    if len <= BITS_IN_CELL:
      write_by_bit_index(path, P_IND, len shr 1, mask(len))
      write_by_bit_index(path, P_IND + (len shr 1), len shr 1, not mask(len))
    else:
      var leftIndex: cuint = (P_IND div BITS_IN_CELL)
      var rightIndex: cuint = leftIndex + (len div BITS_IN_CELL) - 1
      while leftIndex < rightIndex:
        P_DATA[inc(leftIndex)] = 0xFFFFFFFF
        P_DATA[dec(rightIndex)] = 0


  halve(path)

proc polar*(path: Path) =
  if PR_LEV >= 3: return
  if not (read_by_bit_index(path, P_IND, 1) and
      not read_by_bit_index(path, P_IND + P_LEN - 1, 1)):
    skip()
  
proc doalc*(path: Path) =
  var new_cell_count: uint64 = 0
  var new_data_pointer: ptr uint64 = nil
  if PR_LEV >= 1: return
  P_ALC = P_ALC shl 1

  if P_ALC <= BITS_IN_CELL: new_cell_count = BITS_IN_CELL div BITS_IN_BYTE
  else: new_cell_count = P_ALC div BITS_IN_BYTE

  new_cell_count = new_cell_count / sizeof(uint64)
  if (new_data_pointer = alloc0(new_cell_count * sizeof(uint64))) == nil:
    printf("Error allocating %d bytes: ", new_cell_count * sizeof(uint64))
    perror("")
    abort()


  if new_cell_count > 1:


    memcpy(new_data_pointer, P_DATA, new_cell_count * sizeof(uint64) div 2)
  else:
    memcpy(new_data_pointer, P_DATA, sizeof(uint64))

  P_DATA = new_data_pointer

  merge(path)

proc input*(path: Path) =
  var i: cint = P_IND
  if PR_LEV >= 6: return
  if P_LEN < 8:
    write_by_bit_index(path, P_IND, P_LEN, getInput())
    return

  while i < (P_IND + P_LEN):
    write_by_bit_index(path, i, 8, getInput())
    inc(i, 8)

proc algn*(path: Path): uint8 =
  return P_IND mod (P_LEN shl 1) == 0

proc mask*(length: cint): uint64 =
  if length < BITS_IN_CELL: return (cast[cint](1) shl length) - 1
  else: return 0xFFFFFFFF
  
proc read_by_bit_index*(path: Path; i: uint64; len: uint64): uint64 =
  return (P_DATA[i div BITS_IN_CELL] shr
      (BITS_IN_CELL - (i mod BITS_IN_CELL) - len)) and mask(len)

proc write_by_bit_index*(path: Path; i: uint64; len: uint64; write: uint64) =
  var shift: cint = BITS_IN_CELL - (i mod BITS_IN_CELL) - len
  if len > BITS_IN_CELL: abort()
  P_DATA[i div BITS_IN_CELL] = P_DATA[i div BITS_IN_CELL] and
      not (mask(len) shl shift)
  P_DATA[i div BITS_IN_CELL] = P_DATA[i div BITS_IN_CELL] or
      ((write and mask(len)) shl shift)

proc skip*() =
  if P_RUNNING == nil: return
  inc((P_RUNNING.prg_index))







##  Run argv[0] as code. Input separated by '!' and once empty reads the null character.
when isMainModule:

  var args = os.commandLineParams()

  echo args
  while i[] and i[] != INPUT_DELIMITER: inc(i)

  ##  If it is the input delimiter then put the inputptr there
  if i[] == INPUT_DELIMITER: inputptr = inc(i)

  interpret(argv[1])
  return 0