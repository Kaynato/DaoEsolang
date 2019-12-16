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
##  DaoLanguage / Daoyu Utility. (Nim)
## 
##  Kaynato - 2019
## 
## 
## daoyutil.c
## Performs conversions:
##     plaintext -> daoyu source hex
##      > daoyutil <input>
##     daoyu src -> daoyu needed to build
##      > daoyutil <input> -o
## 

import os
import parseopt

import lists
import deques
import sequtils



proc printHelp() =
  echo "Argument syntax: [main args] [command] [command args]"
  echo ""
  echo "Main args:"

  echo "    "
  echo "    (--version / -v)"
  echo "        Prints the version message."

  echo ""
  echo "Commands:"

  echo "    "
  echo "    (help)"
  echo "        Prints the helpfile message."

  echo "    "
  echo "    (bytes) <sym/hex>"
  echo "        Translate input until EOF into Daoyu symbols or hexadecimal plaintext which encode its bytes."
  echo "        Passing a file into STDIN (e.g. via pipe) is recommended."
  echo "        "
  echo "        `sym`"
  echo "            In `sym` mode, the input bytes will be translated into daoyu symbols."
  echo "            e.g. !()[/#$S*=%"
  echo "        "
  echo "        `hex`"
  echo "            In `hex` mode, the input bytes will be translated into hexadecimal plaintext."
  echo "            e.g. 0123456789ABCDEF"

  echo "    "
  echo "    (gen) <sym/hex/raw>"
  echo "        Translate input until EOF from daoyu symbols, hexadecimal plaintext, or raw bytes"
  echo "        Into a daoyu program which writes those bytes."


  # echo "    "
  # echo "    (filter) <bytes>"
  # echo "        "
  # echo "    "

