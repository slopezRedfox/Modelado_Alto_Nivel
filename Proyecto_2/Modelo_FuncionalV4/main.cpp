#include "top.h"

int sc_main(int argc, char* argv[])
{
  Top top("top");
  sc_start(5000, sc_core::SC_NS);
  return 0;
}