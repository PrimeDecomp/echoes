#ifndef _NMWEXCEPTION
#define _NMWEXCEPTION

#include <stddef.h>

#include "__ppc_eabi_linker.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CTORARG_TYPE int
#define CTORARG_PARTIAL (0)
#define CTORARG_COMPLETE (1)

#define CTORCALL_COMPLETE(ctor, objptr)                                                            \
  (((void (*)(void*, CTORARG_TYPE))ctor)(objptr, CTORARG_COMPLETE))

#define DTORARG_TYPE int

#define DTORCALL_COMPLETE(dtor, objptr) (((void (*)(void*, DTORARG_TYPE))dtor)(objptr, -1))
#define DTORCALL_PARTIAL(dtor, objptr) (((void (*)(void*, DTORARG_TYPE))dtor)(objptr, 0))

typedef short vbase_ctor_arg_type;
typedef char local_cond_type;

typedef struct CatchInfo {
  void* location;
  void* typeinfo;
  void* dtor;
  void* sublocation;
  long pointercopy;
  void* stacktop;
} CatchInfo;

typedef struct DestructorChain {
  struct DestructorChain* next;
  void* destructor;
  void* object;
} DestructorChain;

void __end__catch(CatchInfo* catchinfo);
void __throw(char* throwtype, void* location, void* dtor);
char __throw_catch_compare(const char* throwtype, const char* catchtype, long* offset_result);
void __unexpected(CatchInfo* catchinfo);

void __unregister_fragment(int fragmentID);
int __register_fragment(struct __eti_init_info* info, char* TOC);
void* __register_global_object(void* object, void* destructor, void* regmem);
void __destroy_global_chain(void);

#ifdef __cplusplus
}
#endif

#endif // _NMWEXCEPTION
