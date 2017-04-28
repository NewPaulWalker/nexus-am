#include <am.h>
#include <npc.h>

_Area _heap;
_Screen _screen;
ulong npc_cycles = 0;
ulong npc_time = 0;

void _trm_init() {
  serial_init();
  memory_init();
}

void _ioe_init() {
  vga_init();
}

void _halt(int code) {
  _putc('P');
  _putc('a');
  _putc('n');
  _putc('i');
  _putc('c');
  _putc('\n');
  while(1);
}

ulong _uptime() {
  return npc_time ++;
}

ulong _cycles(){
  return npc_cycles ++;
}

void memory_init(){
  //probe a memory for heap
  _heap.start = (void *)HEAP_START;
  _heap.end = (void *)HEAP_END;
}

void serial_init(){
  //not to do
}