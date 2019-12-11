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
##  Kaynato - 2019
## 

import dao_base
import dao_errors

const lookup4 = [
  "0000", "0001", "0010", "0011",
  "0100", "0101", "0110", "0111",
  "1000", "1001", "1010", "1011",
  "1100", "1101", "1110", "1111"
]

const lookup2 = [
  "00", "01", "10", "11"
]

template toBin2(c: uint8): string = lookup2[c]
template toBin4(c: uint8): string = lookup4[c]


proc swapsReader*(reader: var Reader) {.inline.} =
  let node = reader.node
  case node.kind:
    of DnkMIX: swap(node.lc, node.rc)
    of Dnk8:
      if reader.pow == 3: # RmNODE
        node.val = (node.val shl 4 and 0xF0'u8) or (node.val shr 4)
      elif reader.pow == 2: # RmPOS
        let bitsFromRight = 4 * (1 - reader.idx)
        let writeMask = 0b1111'u8 shl bitsFromRight
        let shifted = (node.val shl 2 and 0b11001100'u8) or (node.val shr 2 and 0b00110011'u8)
        node.val = (node.val and not writeMask) or (shifted and writeMask)
      elif reader.pow == 1: # RmPOS
        let bitsFromRight = 2 * (3 - reader.idx)
        let writeMask = 0b11'u8 shl bitsFromRight
        let shifted = (node.val shl 1 and 0b10101010'u8) or (node.val shr 1 and 0b01010101'u8)
        node.val = (node.val and not writeMask) or (shifted and writeMask)
      elif node.pow == 0: # RmPOS, swap bit is discard
        discard
      else:
        Unreachable("SWAPS: Impossible Dnk8 pow.")
    else: discard

proc halveReader*(reader: var Reader) {.inline.} =
  let node = reader.node
  case reader.mode:
  of RmNODE:
    case node.kind:
      of DnkMIX:
        reader.node = node.lc
        dec reader.pow
      of Dnk8, DnkPOS, DnkNEG:
        let child = reader.path.child
        if node.pow == 0 and child != nil:
          ## Descend into bit -> Select child path
          reader.node = child.dataRoot
          reader.path = child
          reader.mode = RmNODE
          reader.pow = reader.node.pow
        else:
          ## Enter the node
          reader.pow = node.pow - 1
          reader.idx = 0
          reader.mode = RmPOS
  of RmPOS:
    if reader.pow == 0:
      ## Descend into bit -> Select child path
      let child = reader.path.child
      if child != nil:
        reader.node = child.dataRoot
        reader.path = child
        reader.mode = RmNODE
    else:
      dec reader.pow
      reader.idx = reader.idx shl 1

proc mergeReader*(reader: var Reader) {.inline.} =
  case reader.mode:
    of RmNODE:
      if reader.node.parent != nil:
        reader.node = reader.node.parent
        reader.pow = reader.node.pow
      else:
        Todo("Select first bit of upper program.")
    of RmPOS:
      if reader.pow == reader.node.pow:
        # Only should happen when inside a Dnk8 of pow < 3.
        if reader.node.parent == nil and reader.path.owner != nil:
          # Try to go to the first BIT of the upper program.
          Todo("Select first bit of upper program.")
        else:
          Unreachable("MERGE: RmPOS Pow match but parent is not nil.")
      else:
        reader.pow += 1
        reader.idx = reader.idx div 2
        if reader.pow > 2 and reader.pow == reader.node.pow:
          reader.mode = RmNODE

proc laterReader*(reader: var Reader) {.inline.} =
  case reader.mode:
    of RmNODE:
      if reader.node.parent == nil or reader.node == reader.node.parent.rc:
        # If root node or right node, defer to merge
        mergeReader(reader)
      else:
        if reader.node == reader.node.parent.lc:
          reader.node = reader.node.parent.rc
          reader.pow = reader.node.pow
        else:
          Unreachable("LATER: Unchained node.")
    of RmPOS:
      # If on the second virtual half of some node, merge
      if (reader.idx and 1'u8) != 0: mergeReader(reader)
      # Otherwise, go to the right half
      else: inc reader.idx

proc readsReader*(reader: var Reader) {.inline.} =
  let node = reader.node
  case reader.mode:
  of RmNODE:
    case node.kind:
      of DnkMIX:
        Todo("Need linear traversal to be implemented.")
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
        for _ in reader.idx ..< (1'u64 shl (reader.pow - 3)):
          stdout.write val
      of Dnk8:
        if reader.pow == 0:   stdout.write "01"[node.val shr (8 - reader.idx) and 1'u8]
        elif reader.pow == 1: stdout.write (node.val shr (4 - reader.idx) and 0b11'u8).toBin2
        elif reader.pow == 2: stdout.write (node.val shr (2 - reader.idx) and 0xF'u8).toBin4
        else: Unreachable("READS: Impossible reader pow.")

proc splitReader*(reader: var Reader) {.inline.} =
  let node = reader.node
  case reader.mode:
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
      reader.node = newNode
    of Dnk8:
      node.val = 0xF0'u8
  of RmPOS:
    case node.kind:
      of Dnk8:
        if reader.pow == 2:
          node.val = node.val and (0x0F'u8 shl (4 * reader.idx))
          node.val = node.val or (0b1100'u8 shl (4 * (1 - reader.idx)))
        elif reader.pow == 1:
          let bitsFromRight = 2 * (3 - reader.idx)
          node.val = node.val and not (0b11'u8 shl bitsFromRight)
          node.val = node.val or (0b10'u8 shl bitsFromRight)
        elif reader.pow == 0:
          discard # Splitting a bit is a noop
        else:
          Unreachable("SPLIT: Impossible Reader pow.")
      of DnkNEG:
        Todo("Heterogenize DnkNEG Interior")
      of DnkPOS:
        Todo("Heterogenize DnkPOS Interior")
      of DnkMIX: Unreachable("SPLIT: POS mode inside DnkMIX")

proc doalcReader*(reader: var Reader) {.inline.} =
  let root = reader.path.dataRoot
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
    reader.path.dataRoot = parent
  mergeReader(reader)
