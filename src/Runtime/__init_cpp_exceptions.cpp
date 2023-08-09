#include "NMWException.h"
#include "__ppc_eabi_linker.h"

#if __MWERKS__
#pragma exceptions off
#pragma internal on
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void __init_cpp_exceptions(void);
extern void __fini_cpp_exceptions(void);

#ifdef __cplusplus
}
#endif

static int fragmentID = -2;

void __exception_info_constants(void** info, char** R2) {
  register char* temp;
  /* clang-format off */
	asm {
		mr      temp,r2
	}
  /* clang-format on */
  *R2 = temp;
  *info = _eti_init_info;
}

// UNUSED: __find_exception_addresses__FPvPcPPvPPv

void __init_cpp_exceptions(void) {
  char* R2;
  void* info;
  if (fragmentID == -2) {
    __exception_info_constants(&info, &R2);
    fragmentID = __register_fragment((struct __eti_init_info*)info, R2);
  }
}

void __fini_cpp_exceptions(void) {
  if (fragmentID != -2) {
    __unregister_fragment(fragmentID);
    fragmentID = -2;
  }
}

/* clang-format off */
#pragma section ".ctors$10"
__declspec(section ".ctors$10")
extern void* const __init_cpp_exceptions_reference = __init_cpp_exceptions;
#pragma section ".dtors$10"
__declspec(section ".dtors$10")
extern void* const __destroy_global_chain_reference = __destroy_global_chain;
#pragma section ".dtors$15"
__declspec(section ".dtors$15")
extern void* const __fini_cpp_exceptions_reference = __fini_cpp_exceptions;
/* clang-format on */
