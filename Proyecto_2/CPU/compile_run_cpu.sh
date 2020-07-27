#!/bin/sh
echo 'Compiling *.c *cpp files'
rm -rf memory.o
export SYSTEMC_HOME=/usr/local/systemc-2.3.3/
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64
g++ -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 cpu_tb.cpp cpu.cpp  -lsystemc -lm -o cpu.o
echo 'Simulation Started'
./cpu.o
echo 'Simulation Ended'