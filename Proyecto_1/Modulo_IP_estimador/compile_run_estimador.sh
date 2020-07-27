#!/bin/sh
echo 'Compiling *.c *cpp files'

rm -rf estimador.o PARAMS.CSV SIGNALS.CSV

export SYSTEMC_HOME=/usr/local/systemc-2.3.3/

export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64

g++ -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 estimador_tb.cpp -lsystemc -lm -o estimador.o

echo 'Simulation Started'

./estimador.o

echo 'Simulation Ended'