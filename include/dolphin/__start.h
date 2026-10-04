#ifndef _DOLPHIN__START
#define _DOLPHIN__START

#include "dolphin/db.h"
#include "types.h"

#ifdef __MWERKS__
#define DECL_SECTION(name) __declspec(section name)
#else
#define DECL_SECTION(name)
#endif

#define PAD3_BUTTON_ADDR 0x800030E4
#define OS_RESET_RESTART 0
#define EXCEPTIONMASK_ADDR 0x80000044
#define BOOTINFO2_ADDR 0x800000F4
#define OS_BI2_DEBUGFLAG_OFFSET 0xC
#define ARENAHI_ADDR 0x80000034
#define DEBUGFLAG_ADDR 0x800030E8
#define DVD_DEVICECODE_ADDR 0x800030E6

#define MSR_FP 0x2000

extern void InitMetroTRK();

#ifdef __MWERKS__
u16 Pad3Button : PAD3_BUTTON_ADDR;
#else
u16 Pad3Button;
#endif
static u8 Debug_BBA = 0;

extern void memset(void*, int, int);
extern int main(int argc, char* argv[]);
extern void exit(int);
extern void __init_user(void);
extern void OSInit(void);
extern void DBInit(void);
extern void OSResetSystem(BOOL reset, u32 resetCode, BOOL forceMenu);
extern void __OSCacheInit(void);
extern void __OSPSInit(void);

DECL_SECTION(".init") extern void __check_pad3(void);
DECL_SECTION(".init") static void __set_debug_bba(void);
DECL_SECTION(".init") static u8 __get_debug_bba(void);
DECL_SECTION(".init") extern void __start(void);
DECL_SECTION(".init") extern void __init_registers(void);
DECL_SECTION(".init") extern void __init_data(void);
DECL_SECTION(".init") extern void __init_hardware(void);
DECL_SECTION(".init") extern void __flush_cache(void* address, unsigned int size);

DECL_SECTION(".init") extern char _stack_addr[];
DECL_SECTION(".init") extern char _SDA_BASE_[];
DECL_SECTION(".init") extern char _SDA2_BASE_[];

#endif // _DOLPHIN__START
