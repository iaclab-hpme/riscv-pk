#include "plugins/plugins.h"

#ifdef PLUGIN_ENABLE_MULTIMEM
  #include "plugins/multimem.c"
#endif

#ifdef PLUGIN_ENABLE_HPME
  #include "plugins/hpme.c"
#endif

uintptr_t
call_plugin(
    enclave_id id,
    uintptr_t plugin_id,
    uintptr_t call_id,
    uintptr_t arg0,
    uintptr_t arg1,
    uintptr_t arg2,
    uintptr_t arg3)
{
  switch(plugin_id) {
#ifdef PLUGIN_ENABLE_MULTIMEM
    case PLUGIN_ID_MULTIMEM:
      return do_sbi_multimem(id, call_id);
      break;
#endif
#ifdef PLUGIN_ENABLE_HPME
    case PLUGIN_ID_HPME:
      return do_sbi_hpme(id, call_id, arg0, arg1, arg2, arg3);
#endif
    default:
      // TOO fix it
      return -ENOSYS;
  }
}

