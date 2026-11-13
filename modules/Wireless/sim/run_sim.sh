#!/usr/bin/env bash
# Desktop simulation of the Wireless module test sketch.
# Compiles the REAL Wireless.cpp and Wireless.ino against stub radio/Arduino
# headers, producing two programs: node and host.
#
# Usage:
#   ./run_sim.sh build          # compile both
#   ./run_sim.sh host           # run in terminal 1
#   ./run_sim.sh node           # run in terminal 2
#   SIM_LOSS=0.3 ./run_sim.sh node   # optional: drop 30% of node packets
set -e
cd "$(dirname "$0")"
SRC=..
CXXFLAGS="-std=c++17 -O1 -I. -I$SRC"

build() {
  printf '#include "SX126x-Arduino.h"\nvoid setup(); void loop();\nint main(){ setup(); for(;;){ loop(); delay(1); } }\n' > main_sim.cpp
  g++ $CXXFLAGS -DROLE=1 -DNODE_ID=1 -DSIM_MY_PORT=47001 -DSIM_PEER_PORT=47002 \
      -x c++ $SRC/Wireless.ino -x c++ $SRC/Wireless.cpp main_sim.cpp -o node
  g++ $CXXFLAGS -DROLE=2 -DSIM_MY_PORT=47002 -DSIM_PEER_PORT=47001 \
      -x c++ $SRC/Wireless.ino -x c++ $SRC/Wireless.cpp main_sim.cpp -o host
  echo "Built ./node and ./host"
}

case "$1" in
  build) build ;;
  node)  [ -x node ] || build; exec ./node ;;
  host)  [ -x host ] || build; exec ./host ;;
  *) echo "usage: $0 build|node|host"; exit 1 ;;
esac
