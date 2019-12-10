const lookup4* = [
  "0000", "0001", "0010", "0011",
  "0100", "0101", "0110", "0111",
  "1000", "1001", "1010", "1011",
  "1100", "1101", "1110", "1111"
]

const lookup2* = [
  "00", "01", "10", "11"
]

template toBin2*(c: uint8): string = lookup2[c]
template toBin4*(c: uint8): string = lookup4[c]
