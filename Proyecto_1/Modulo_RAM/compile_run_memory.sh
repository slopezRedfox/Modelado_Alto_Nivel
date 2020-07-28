#!/bin/sh
echo 'Compiling *.c *cpp files'
rm -rf memory.o
export SYSTEMC_HOME=/usr/local/systemc-2.3.3/
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64
g++ -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 dram_tb.cpp dram.cpp  -lsystemc -lm -o dram.o
echo 'Simulation Started'
./dram.o
echo 'Simulation Ended'