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

template toBin2*(c: uint8): string = lookup2[c]
template toBin4*(c: uint8): string = lookup4[c]

# Forward declare
proc halveReader*(reader: var Reader) {.inline.}
proc hlvrtReader*(reader: var Reader) {.inline.}

proc store*(reader: Reader): StoredReader =
  result.path = reader.path
  result.rootPow = reader.path.dataRoot.pow
  result.pow = reader.pow

  if result.rootPow == result.pow:
    return result

  var pow = reader.pow
  var idx = reader.idx
  var nMoves: int
  var move: uint64
  # Go up until we stop being in POS mode or meet the root
  while pow < reader.node.pow and pow < result.rootPow:
    # Push lc/rc to the move and ascend
    move = (move shl 1) or (idx and 1)
    idx = idx div 2
    inc pow
    inc nMoves
    inc result.nBitsInFirstMove
    if nMoves >= 64:
      nMoves -= 64
      result.moves.add(move)
      move = 0'u64
      result.nBitsInFirstMove = 0'u64

  # Go up until we meet the pow
  var node = reader.node
  # echo pow, " ", result.rootPow
  while pow < result.rootPow:
    if node.parent == nil:
      echo "STORE READER: Parent of ", $(node), " at pow ", pow, " was nil but rootPow was ", result.rootPow
      assert false
    if node == node.parent.lc:
      move = (move shl 1)
    elif node == node.parent.rc:
      move = (move shl 1) or 1'u64
    else:
      Unreachable("STORE: Hanging node")
    node = node.parent
    inc pow
    inc nMoves
    inc result.nBitsInFirstMove
    if nMoves >= 64:
      nMoves -= 64
      result.moves.add(move)
      move = 0'u64
      result.nBitsInFirstMove = 0'u64

  # Add moves if remaining
  if nMoves > 0:
    result.moves.add(move)
    # echo "put ", move, " in moves -> ", result.moves
    result.nBitsInFirstMove = nMoves.uint64

proc retrieve*(stored: StoredReader): Reader =
  result.path = stored.path
  if result.path == nil or result.path.dataRoot == nil: return result

  # Drop down to the root
  result.node = result.path.dataRoot
  result.pow = result.node.pow
  result.idx = 0
  result.mode = if result.pow >= 3'u64: RmNODE else: RmPOS

  # No moves? Ok, then.
  if stored.moves.len == 0:
    return result

  var moves = stored.moves
  var nBitsInFirstMove = stored.nBitsInFirstMove
  var move = moves.pop()
  # If path became smaller while we were gone, we need to ignore some moves
  if result.node.pow < stored.rootPow:
    # This is how many moves we should ignore.
    var powdiff = stored.rootPow - result.node.pow
    if powdiff >= nBitsInFirstMove:
      # Completely ignore the first move possibly with room to spare
      powdiff -= nBitsInFirstMove
      nBitsInFirstMove = 64
      if moves.len > 0:
        move = moves.pop()
      else:
        return result
    while powdiff >= 64'u64:
      powdiff -= 64
      if moves.len > 0:
        move = moves.pop()
      else:
        return result
    # Powdiff is now in the range 0..63
    move = move shr powdiff
    nBitsInFirstMove -= powdiff
  # What if the path became bigger while we were gone?
  while result.node.pow > stored.rootPow:
    # Honestly, that's fine. We just need to drop down to the stored pow.
    halveReader(result)
  # Now the reader is at the rootPow, or the correct amount of moves has been ignored.
  # Either way, we are now ready to consume the moves.
  while nBitsInFirstMove > 0'u64:
    if (move and 1'u64) == 0:
      halveReader(result)
    else:
      hlvrtReader(result)
    move = move shr 1
    dec nBitsInFirstMove
  while moves.len > 0:
    move = moves.pop()
    for _ in 0..63:
      if (move and 1'u64) == 0:
        halveReader(result)
      else:
        hlvrtReader(result)
      move = move shr 1
  # Strange things may have happened. Set mode again
  if result.pow < 3'u64:
    result.mode = RmPOS

proc descendPath*(reader: var Reader) {.inline.} =
  ## Select full child path
  let child = reader.path.child
  if child != nil and child.dataRoot != nil:
    reader.node = child.dataRoot
    reader.path = child
    reader.mode = if reader.node.pow >= 3'u64: RmNODE else: RmPOS
    reader.pow = reader.node.pow
    reader.idx = 0

proc ascendPath*(reader: var Reader) {.inline.} =
  let owner = reader.path.owner
  if owner != nil and owner.dataRoot != nil:
    reader.node = owner.dataRoot
    reader.path = owner
    reader.mode = if reader.node.pow >= 3'u64: RmNODE else: RmPOS
    reader.pow = reader.node.pow
    reader.idx = 0
    while reader.pow > 0'u64:
      halveReader(reader)

proc getHex*(reader: Reader): uint8 {.inline.} =
  if reader.pow != 2: Unreachable("getHex: Reader pow not 2")
  let node = reader.node
  case node.kind:
    of DnkPOS: return 0xF'u8
    of DnkNEG: return 0x0'u8
    of Dnk8: return (if reader.idx == 0: node.val shr 4 else: node.val and 0xF'u8)
    of DnkMIX: Unreachable("getHex: DnkMIX cannot be pow2")

proc getByte*(reader: Reader): uint8 {.inline.} =
  if reader.pow != 3: Unreachable("getByte: Reader pow not 3 but was " & $(reader.pow))
  let node = reader.node
  case node.kind:
    of DnkPOS: return 0xFF'u8
    of DnkNEG: return 0x00'u8
    of Dnk8: return node.val
    of DnkMIX: Unreachable("getHex: DnkMIX cannot be pow3")

proc justReplaceCurrentNode*(reader: var Reader, newNode: DaoNode, setReaderNode: static bool) =
  ## Just replace the current node. Won't replace pow or idx or mode.
  ## If setReaderNode set to true, also replaces the reader node with the newNode.
  let oldNode = reader.node
  if oldNode == reader.path.dataRoot:
    # Assign new node to path root if need be
    reader.path.dataRoot = newNode
  else:
    # If we weren't root
    # Need to set our parent
    if oldNode.parent == nil:
      Unreachable("REPLACE: Non-root node with nil parent")
    assert oldnode.parent.kind == DnkMIX, "REPLACE: Somehow you had nonmix parent"
    newNode.parent = oldNode.parent
    if oldNode == oldNode.parent.lc:
      oldNode.parent.lc = newNode
    elif oldNode == oldNode.parent.rc:
      oldNode.parent.rc = newNode
    else:
      Unreachable("REPLACE: Hanging node!")
  when setReaderNode:
    reader.node = newNode

proc climbTo(curr, target: DaoNode; idx: uint64): DaoNode = 
  ## For writing to the interior of pure nodes.
  ## From curr, make nodes that go up until target is reached.
  result = curr
  var idx = idx
  let kind = target.kind
  let defaultValue = if kind == DnkPOS: 0xFF'u8 else: 0x00'u8
  # Reuse logic from retrieve to descend from the target node
  while result.pow < target.pow:
    # Create a parent and a sibling
    let is_rc = (idx and 1'u64) > 0'u64
    var parent = DaoNode(kind: DnkMIX, parent: nil, pow: result.pow + 1)
    result.parent = parent

    let sibling =
      if result.pow > 3: DaoNode(kind: kind, parent: parent, pow: result.pow)
      else:              DaoNode(kind: Dnk8, parent: parent, pow: result.pow, val: defaultValue)

    if is_rc:
      parent.rc = result
      parent.lc = sibling
    else: # is lc
      parent.rc = sibling
      parent.lc = result

    result = parent
    idx = idx shr 1


proc leftmostBit*(reader: Reader): uint8 {.inline.} =
  var lc = reader.node
  while lc.kind == DnkMIX:
    lc = lc.lc
  # Not polar if left child is DnkNEG
  if lc.kind == DnkNEG:
    return 0'u8
  # Early exits on Dnk8 if lc leftmost bit is zero
  if lc.kind == Dnk8:
    if lc.pow > 3'u64:
      Unreachable("LEFTMOSTBIT: Impossible lc pow")
    else:
      let pow: range[0..3] = (if reader.pow > 3'u64: 3 else: reader.pow.int)
      return case pow:
        of 3:           lc.val shr  7
        of 2: 1'u8 and (lc.val shr (7 - (reader.idx shl 2)))
        of 1: 1'u8 and (lc.val shr (7 - (reader.idx shl 1)))
        of 0: 1'u8 and (lc.val shr (7 -  reader.idx       ))
  # DnkPOS
  return 1'u8

proc rightmostBit*(reader: Reader): uint8 {.inline.} =
  var rc = reader.node
  while rc.kind == DnkMIX:
    rc = rc.rc
  if rc.kind == DnkPOS:
    return 1'u8
  if rc.kind == Dnk8:
    if rc.pow > 3'u64:
      Unreachable("LEFTMOSTBIT: Impossible rc pow")
    else:
      let pow: range[0..3] = (if reader.pow > 3'u64: 3 else: reader.pow.int)
      return case pow:
        of 3: 1'u8 and  rc.val
        of 2: 1'u8 and (rc.val shr (4 - (reader.idx shl 2)))
        of 1: 1'u8 and (rc.val shr (6 - (reader.idx shl 1)))
        of 0: 1'u8 and (rc.val shr (7 -  reader.idx       ))
  return 0'u8

proc swapsReader*(reader: Reader) {.inline.} =
  let node = reader.node
  case node.kind:
    of DnkMIX: swap(node.lc, node.rc)
    of Dnk8:
      if reader.pow == 3: # RmNODE
        node.val = (node.val shl 4 and 0xF0'u8) or (node.val shr 4)
      elif reader.pow == 2: # RmPOS
        let bitsFromRight = (1 - reader.idx.int) shl 2
        let writeMask = 0b1111'u8 shl bitsFromRight
        let shifted = (node.val shl 2 and 0b11001100'u8) or (node.val shr 2 and 0b00110011'u8)
        node.val = (node.val and not writeMask) or (shifted and writeMask)
      elif reader.pow == 1: # RmPOS
        let bitsFromRight = (3 - reader.idx.int) shl 1
        let writeMask = 0b11'u8 shl bitsFromRight
        let shifted = (node.val shl 1 and 0b10101010'u8) or (node.val shr 1 and 0b01010101'u8)
        node.val = (node.val and not writeMask) or (shifted and writeMask)
      elif reader.pow == 0: # RmPOS, swap bit is discard
        discard
      else:
        Unreachable("SWAPS: Impossible Dnk8 pow: " & $(reader.pow))
    else: discard

proc halveReader*(reader: var Reader) {.inline.} =
  ## Go to left child
  let node = reader.node
  case reader.mode:
  of RmNODE:
    case node.kind:
      of DnkMIX:
        reader.node = node.lc
        dec reader.pow
      of Dnk8, DnkPOS, DnkNEG:
        if node.pow == 0:
          descendPath(reader)
        else:
          ## Enter the node
          reader.pow = node.pow - 1
          reader.idx = 0
          reader.mode = RmPOS
  of RmPOS:
    if reader.pow == 0:
      descendPath(reader)
    else:
      dec reader.pow
      reader.idx = reader.idx shl 1
      if node.pow - reader.pow >= 64:
        Todo("HALVE: Descending will make the reader idx overflow. But you probably don't want 2.25 Exabytes anyway.")

proc hlvrtReader*(reader: var Reader) {.inline.} =
  ## Go to right child
  let node = reader.node
  case reader.mode:
  of RmNODE:
    case node.kind:
      of DnkMIX:
        reader.node = node.rc
        dec reader.pow
      of Dnk8, DnkPOS, DnkNEG:
        if node.pow == 0:
          descendPath(reader)
        else:
          ## Enter the node
          reader.pow = node.pow - 1
          reader.idx = 1
          reader.mode = RmPOS
  of RmPOS:
    if reader.pow == 0:
      descendPath(reader)
    else:
      dec reader.pow
      reader.idx = (reader.idx shl 1) + 1'u64


proc mergeReader*(reader: var Reader, allow_ascent: static bool = true): bool {.inline.} =
  ## Move the reader to parent if possible.
  ## If no parent and allow_ascent, try to move to the parent path's first bit.
  ## Return true if merge succeeded without ascent.
  case reader.mode:
    of RmNODE:
      if reader.node.parent != nil:
        reader.node = reader.node.parent
        reader.pow = reader.node.pow
        return true
      else:
        when allow_ascent:
          ascendPath(reader)
          return false
        else:
          return false
    of RmPOS:
      if reader.pow == reader.node.pow:
        # Only should happen when inside a Dnk8 of pow < 3. Means we're at root.
        when allow_ascent:
          if reader.node.parent == nil and reader.path.owner != nil:
            # Try to go to the first BIT of the upper program.
             ascendPath(reader)
          else:
            Unreachable("MERGE: RmPOS Pow match but parent is not nil.")
        return false
      else:
        reader.pow += 1
        reader.idx = reader.idx div 2
        if reader.pow > 2'u64 and reader.pow == reader.node.pow:
          reader.mode = RmNODE
        return true

proc laterReader*(reader: var Reader, allow_merge: static bool = true): bool {.inline.} =
  ## If lc, move to rc. If rc, go to parent. Returns whether we moved or not (e.g. if was at top)
  case reader.mode:
    of RmNODE:
      if reader.node.parent == nil or reader.node == reader.node.parent.rc:
        # If root node or right node, defer to merge
        when allow_merge:
          discard mergeReader(reader)
        return false
      else:
        if reader.node == reader.node.parent.lc:
          reader.node = reader.node.parent.rc
          reader.pow = reader.node.pow
          return true
        else:
          Unreachable("LATER: Unchained node.")
    of RmPOS:
      if ((reader.idx and 1'u8) != 0) or (reader.node.pow == 0):
        # If on the second virtual half of some node, or root is bit, merge
        discard mergeReader(reader)
      # Otherwise, go to the right half
      else: inc reader.idx
      return true

proc linesReader*(reader: var Reader): bool {.inline.} =
  ## Move right once and return true. If can't move right, return false.
  # Move to next linear position and return the value there.
  let origPow = reader.pow
  if reader.mode == RmNODE:
    # If we're left child, go to right child
    if reader.node == reader.node.parent.lc:
      reader.node = reader.node.parent.rc
      return true
  else:
    # We are certainly inside of DnkPOS or DnkNEG.
    let pdiff = reader.node.pow - reader.pow
    if pdiff >= 64'u64:
      Todo("moveThenGet: Index might overflow uint64")
    # If we can move right, just move and get the value.
    if (((reader.idx + 1) shr pdiff) and 1'u64) == 0:
      # Move right and get value
      inc reader.idx
      return true
    # Otherwise, we have to actually merge out of the node itself
    reader.mode = RmNODE
    reader.pow = reader.node.pow
    reader.idx = 0
  # Coming out here means we need to come out of the node :(
  # No parent or was right child
  if reader.node.parent == nil:
    return false
  # Was right child - go up until is left child or hits root node
  while reader.node != reader.node.parent.lc:
    discard mergeReader(reader, allow_ascent=false)
    if reader.node.parent == nil:
      return false
  # Now we are the left child, so move right
  if not laterReader(reader, allow_merge=false):
    Unreachable("moveThenGet: Tried to merge despite being left child.")
  # Move down until pow is back to before
  while reader.pow > origPow:
    halveReader(reader)
  # Now we're good
  return true

proc readsReader*(reader: var Reader) {.inline.} =
  let node = reader.node
  case reader.mode:
  of RmNODE:
    case node.kind:
      of DnkMIX:
        # Copy reader for linear traversal and move to pow 3
        var readHead = reader
        while readHead.pow > 3'u64: halveReader(readHead)
        stdout.write readHead.getByte().char
        while linesReader(readHead): stdout.write readHead.getByte().char
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
        if reader.pow == 0:   stdout.write "01"[node.val shr (8'u64 - reader.idx) and 1'u8]
        elif reader.pow == 1: stdout.write (node.val shr (4'u64 - reader.idx) and 0b11'u8).toBin2
        elif reader.pow == 2: stdout.write (node.val shr (2'u64 - reader.idx) and 0xF'u8).toBin4
        else: Unreachable("READS: Impossible reader pow.")

proc inputReader*(reader: var Reader, inputStream: File) {.inline.} =
  if reader.pow > 3'u64:
    # More than one byte
    var charbuf = newSeq[uint8](1 shl (reader.pow - 3))
    try:
      discard inputStream.readBytes(charbuf, 0, charbuf.len)
    except EOFError:
      # EOF -> Read EOFs (-1)
      for ch in charbuf.mitems:
        ch = 0xFF'u8
    let inputTree = treeify(charbuf)
    # '\' 92 '\r' 13 '\n' 10
    # What a terrible night to have a curse... input via human command line input always contaminated with endline
    case reader.mode:
      of RmNODE:
        reader.justReplaceCurrentNode(inputTree.root, setReaderNode=true)
      of RmPOS:
        case reader.node.kind:
        of DnkPOS, DnkNEG:
          # Write the new subtree in the interior of the pure node
          let replacement = inputTree.root.climbTo(reader.node, reader.idx)
          # Now connect replacement to where the reader was
          # Pow unchanged
          reader.justReplaceCurrentNode(replacement, setReaderNode=false)
          reader.node = inputTree.root
          reader.mode = RmNODE
          reader.idx = 0
        of Dnk8:
          Unreachable("INPUT: POS mode Dnk8 despite having checked pow > 3")
        of DnkMIX:
          Unreachable("INPUT: POS mode inside DnkMIX")
  else:
    # One byte or less
    let ival =
      try:
        inputStream.readChar().uint8
      except EOFError:
        0xFF'u8 # EOF (-1)

    case reader.node.kind:
    of Dnk8:
      reader.node.val =
        if reader.pow == 3:      iVal
        elif reader.pow == 2:    iVal and 0x0F'u8
        elif reader.pow == 1:    iVal and 0x03'u8
        elif reader.pow == 0:    iVal and 0x01'u8
        else: Unreachable("INPUT: Impossible Dnk8 pow: " & $(reader.pow))
    of DnkPOS, DnkNEG:
      var curr: DaoNode
      var idx, sub3Idx: uint64
      if reader.pow > 3:
        Unreachable("INPUT: Reader.pow > 3 though we just checked the converse.")
      elif reader.pow == 3:
        curr = DaoNode(kind: Dnk8, parent: curr, pow: 3, val: iVal)
        # Preemptive setting
        reader.mode = RmNODE
        reader.idx = 0
      else:
        # Need to know idx too. We know reader.node.pow > 3.
        # Ok to make curr pow 3 as long as we adjust virtual idx accordingly.
        let writeValue = iVal and (1'u8 shl (1 shl reader.pow.int) - 1'u8)
        let pdiff = 3 - reader.pow.int
        let idxMask = (1'u64 shl pdiff) - 1'u64 # Grabs the relevant part of the index
        sub3Idx = reader.idx and idxMask
        let bitsFromRight = (idxMask - sub3Idx) shl reader.pow

        let val =
          if reader.node.kind == DnkNEG:     writeValue  shl bitsFromRight
          else:                    not ((not writeValue) shl bitsFromRight)
        curr = DaoNode(kind: Dnk8, parent: nil, pow: 3, val: val)
        idx = reader.idx shr pdiff
        # Preemptive setting
        reader.idx = sub3Idx

      let climber = curr.climbTo(reader.node, idx)
      reader.justReplaceCurrentNode(climber, setReaderNode=false)
      reader.node = curr
    of DnkMIX:
      Unreachable("INPUT: Under pow 3 in DnkMIX")

proc splitReader*(reader: var Reader) {.inline.} =
  let node = reader.node
  case reader.mode:
  of RmNODE:
    case node.kind:
    of DnkMIX:
      if node.pow > 4'u64:
        if node.lc.kind != DnkPOS: node.lc = DaoNode(kind: DnkPOS, parent: node, pow: node.pow - 1)
        if node.rc.kind != DnkNEG: node.rc = DaoNode(kind: DnkNEG, parent: node, pow: node.pow - 1)
      elif node.pow == 4'u64:
        if node.lc.kind != DnkPOS: node.lc = DaoNode(kind: Dnk8, parent: node, pow: 3, val: 0xFF'u8)
        if node.rc.kind != DnkNEG: node.rc = DaoNode(kind: Dnk8, parent: node, pow: 3, val: 0x00'u8)
      else: Unreachable("SPLIT: Illegal DnkMIX of pow <= 3")
    of DnkNEG, DnkPOS:
      # Overwrite the node
      let newNode = DaoNode(kind: DnkMIX, parent: node.parent, pow: node.pow, lc: nil, rc: nil)
      if node.pow > 4'u64:
        newNode.lc = DaoNode(kind: DnkPOS, parent: newNode, pow: node.pow - 1)
        newNode.rc = DaoNode(kind: DnkNEG, parent: newNode, pow: node.pow - 1)
      elif node.pow == 4'u64:
        newNode.lc = DaoNode(kind: Dnk8, parent: newNode, pow: 3, val: 0xFF'u8)
        newNode.rc = DaoNode(kind: Dnk8, parent: newNode, pow: 3, val: 0x00'u8)
      else: Unreachable("SPLIT: Illegal DnkMIX of pow <= 3")
      reader.justReplaceCurrentNode(newNode, setReaderNode=true)
    of Dnk8:
      node.val = 0xF0'u8
  of RmPOS:
    case node.kind:
      of Dnk8:
        if reader.pow == 2:
          node.val = node.val and (0x0F'u8 shl (reader.idx shl 2))
          node.val = node.val or (0b1100'u8 shl ((1'u64 - reader.idx) shl 2))
        elif reader.pow == 1:
          let bitsFromRight = (3'u64 - reader.idx) shl 1
          node.val = node.val and not (0b11'u8 shl bitsFromRight)
          node.val = node.val or (0b10'u8 shl bitsFromRight)
        elif reader.pow == 0:
          discard # Splitting a bit is a noop
        else:
          Unreachable("SPLIT: Impossible Reader pow.")
      of DnkNEG, DnkPOS:
        var curr: DaoNode
        var idx: uint64
        var sub3Idx: uint64
        if reader.pow > 2:
          if reader.pow > 3:
            # If greater than 3, we have to make the parent node and the children too
            var lc, rc: DaoNode
            curr = DaoNode(kind: DnkMIX, parent: nil, pow: reader.pow, lc: nil, rc: nil)
            if reader.pow > 4:
              lc = DaoNode(kind: DnkPOS, parent: curr, pow: reader.pow - 1)
              rc = DaoNode(kind: DnkNEG, parent: curr, pow: reader.pow - 1)
            else: # reader.pow == 4
              lc = DaoNode(kind: Dnk8, parent: curr, pow: 3, val: 0xFF'u8)
              rc = DaoNode(kind: Dnk8, parent: curr, pow: 3, val: 0x00'u8)
            curr.lc = lc
            curr.rc = rc
          else: # reader.pow == 3
            # Otherwise there's a single split child
            curr = DaoNode(kind: Dnk8, parent: curr, pow: 3, val: 0xF0'u8)
          # The node "curr" goes at the current idx
          idx = reader.idx
        elif reader.pow == 2:
          # Need to know idx too. We know reader.node.pow > 3.
          # Ok to make curr pow 3 as long as we adjust virtual idx accordingly.
          # Are we the left child or right child of pow 3 layer?
          let is_rc = (reader.idx and 1'u64) > 0'u64
          let val = if node.kind == DnkNEG:
            if is_rc: 0b00001100'u8 else: 0b11000000'u8
          else:
            if is_rc: 0b11111100'u8 else: 0b11001111'u8
          curr = DaoNode(kind: Dnk8, parent: nil, pow: 3, val: val)
          idx = reader.idx shr 1
          sub3Idx = reader.idx and 0b1'u64
        elif reader.pow == 1:
          # Last two bits tell us about the branching from the pow 3 layer
          let bitsFromRight = (3 - (reader.idx and 0b11'u64)) shl 1
          let val =
            if node.kind == DnkNEG:     (0b10'u8 shl bitsFromRight)
            else:                   not (0b01'u8 shl bitsFromRight)
          curr = DaoNode(kind: Dnk8, parent: nil, pow: 3, val: val)
          idx = reader.idx shr 2
          sub3Idx = reader.idx and 0b11'u64
        elif reader.pow == 0:
          return # nothing happens

        let climber = curr.climbTo(node, idx)

        # Now connect climber to where the reader was
        reader.justReplaceCurrentNode(climber, setReaderNode=false)
        # Then set the reader to curr (or within curr)
        # Pow unchanged
        reader.node = curr
        if reader.pow >= 3:
          reader.mode = RmNODE
          reader.idx = 0
        else: # Sub3
          reader.idx = sub3Idx

      of DnkMIX:
        Unreachable("SPLIT: POS mode inside DnkMIX")

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
        root # parent = ...
      else: Unreachable("DOALC: Impossible Dnk8 pow.")
    of DnkNEG:
      DaoNode(kind: DnkNEG, pow: root.pow + 1, parent: nil)
  # Bind parents ONLY if it's not the special modification case
  if parent.kind != Dnk8:
    if parent.kind == DnkMIX:
      parent.rc.parent = parent
      root.parent = parent
    reader.path.dataRoot = parent
  discard mergeReader(reader)

proc dealcReader*(reader: var Reader) {.inline.} = 
  let root = reader.path.dataRoot
  let stored = reader.store
  case root.kind:
    of DnkMIX:
      reader.path.dataRoot = reader.path.dataRoot.lc
      reader.path.dataRoot.parent = nil
    of DnkPOS, DnkNEG:
      root.pow -= 1
      if root.pow == 3'u64:
        let val = if root.kind == DnkPOS: 0xFF'u8 else: 0'u8
        reader.path.dataRoot = DaoNode(kind: Dnk8, pow: 3, parent: nil, val: val)
    of Dnk8:
      if root.pow > 0'u64:
        root.pow -= 1
        root.val = root.val shr (1 shl root.pow)
      else:
        # Destroy path
        reader.path.dataRoot = nil
  reader = stored.retrieve
