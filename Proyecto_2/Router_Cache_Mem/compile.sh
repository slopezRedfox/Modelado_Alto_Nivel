#!/bin/sh
export exe=exe
export cpp1=main.cpp

export SYSTEMC_HOME=/usr/local/systemc-2.3.3/
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64

echo 'Compiling *.c *cpp files'
rm -rf $exe
echo ''

g++ -I$SYSTEMC_HOME/include -L$LD_LIBRARY_PATH $cpp1  -lsystemc -lm -o $exe

echo ''
echo 'Simulation Started'
./$exe
echo 'Simulation Ended'
