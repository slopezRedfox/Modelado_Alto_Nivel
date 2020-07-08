#!/bin/sh
echo 'Compiling *.c *cpp files'
rm -rf memory
echo ''

export exe=memory
export cpp1=memory.cpp
export cpp2=memory_tb.cpp

export SYSTEMC_HOME=/usr/local/systemc-2.3.2/
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64

g++ -I$SYSTEMC_HOME/include -L$LD_LIBRARY_PATH $cpp2 $cpp1  -lsystemc -lm -o $exe

echo ''
echo 'Simulation Started'
./memory
echo 'Simulation Ended'