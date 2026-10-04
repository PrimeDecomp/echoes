#include <dolphin/__start.h>

static u8 Debug_BBA = 0; // Prime-correlated name.

__declspec(section ".init") static void __check_pad3(void) {}

__declspec(section ".init") static void __set_debug_bba(void) {} // Guessed name.

__declspec(section ".init") static u8 __get_debug_bba(void) {} // Guessed name.

__declspec(weak) asm void __start(void) {}

__declspec(section ".init") static asm void __init_registers(void) {}

__declspec(section ".init") static void __init_data(void) {}
