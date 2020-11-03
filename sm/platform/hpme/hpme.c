//
// Created by yintianyu on 2020/10/13.
//

#include <string.h>
#include "hpme.h"
#include "enclave.h"
#include "pmp.h"

/*
 * Current we don't find any necessary thing to do for scratchpad init.
 */
enclave_ret_code scratch_init(){
  return ENCLAVE_SUCCESS;
}

enclave_ret_code platform_init_global_once(){
  /* Create PMP region for scratchpad */
  if(pmp_region_init_atomic(L2_SCRATCH_START,
                            L2_SCRATCH_STOP - L2_SCRATCH_START,
                            PMP_PRI_ANY, &scratch_rid, 1)){
    printm("FATAL CANNOT CREATE SCRATCH PMP\r\n");
    return ENCLAVE_NO_FREE_RESOURCE;
  }
  return ENCLAVE_SUCCESS;
}

enclave_ret_code platform_init_global(){
  pmp_set(scratch_rid, PMP_NO_PERM);

  return ENCLAVE_SUCCESS;
}

void platform_init_enclave(struct enclave* enclave){
  return;
}

void platform_destroy_enclave(struct enclave* enclave){
  int scratch_epm_idx = get_enclave_region_index(enclave->eid, REGION_EPM);
  /* Clean out the region ourselves */

  /* Should be safe to just write to the memory addresses we used to
     initialize */
  uintptr_t addr;
  uintptr_t scratch_start = pmp_region_get_addr(enclave->regions[scratch_epm_idx].pmp_rid);
  uintptr_t scratch_stop = scratch_start + pmp_region_get_size(enclave->regions[scratch_epm_idx].pmp_rid);
  for( addr = scratch_start;
       addr < scratch_stop;
       addr += sizeof(uintptr_t)){
    *(uintptr_t*)addr = 0;
  }

  /* Fix the enclave region info to no longer know about
     scratchpad */

  enclave->regions[scratch_epm_idx].type = REGION_INVALID;
}

enclave_ret_code platform_create_enclave(struct enclave* enclave){
  // Currently we don't arrange a use_scratch flag, force using scratchpad

  if(scratch_init() != ENCLAVE_SUCCESS){
    return ENCLAVE_UNKNOWN_ERROR;
  }

  /* Swap regions */
  int old_epm_idx = get_enclave_region_index(enclave->eid, REGION_EPM);
  int new_idx = get_enclave_region_index(enclave->eid, REGION_INVALID);
  if(old_epm_idx < 0 || new_idx < 0){
    return ENCLAVE_NO_FREE_RESOURCE;
  }

  enclave->regions[new_idx].pmp_rid = scratch_rid;
  enclave->regions[new_idx].type = REGION_EPM;
  enclave->regions[old_epm_idx].type = REGION_OTHER;

  /* Copy the enclave over */
  uintptr_t old_epm_start = pmp_region_get_addr(enclave->regions[old_epm_idx].pmp_rid);
  uintptr_t scratch_epm_start = pmp_region_get_addr(scratch_rid);
  size_t size = enclave->pa_params.free_base - old_epm_start;
  size_t scratch_size = L2_SIZE;

  if(size > scratch_size){
    printm("FATAL: Enclave too big for scratchpad!\r\n");
    return ENCLAVE_NO_FREE_RESOURCE;
  }
  memcpy((enclave_ret_code*)scratch_epm_start,
         (enclave_ret_code*)old_epm_start,
         size);
  printm("Performing copy from %llx to %llx\r\n", old_epm_start, scratch_epm_start);
  /* Change pa params to the new region */
  enclave->pa_params.dram_base = scratch_epm_start;
  enclave->pa_params.dram_size = scratch_size;
  enclave->pa_params.runtime_base = (scratch_epm_start +
                                     (enclave->pa_params.runtime_base -
                                      old_epm_start));
  enclave->pa_params.user_base = (scratch_epm_start +
                                  (enclave->pa_params.user_base -
                                   old_epm_start));
  enclave->pa_params.free_base = (scratch_epm_start +
                                  size);
  enclave->encl_satp =((scratch_epm_start >> RISCV_PGSHIFT) | SATP_MODE_CHOICE);

  /* printm("[new pa_params]: \r\n\tbase_addr: %llx\r\n\tbasesize: %llx\r\n\truntime_addr: %llx\r\n\tuser_addr: %llx\r\n\tfree_addr: %llx\r\n", */
  /*        enclave->pa_params.dram_base, */
  /*        enclave->pa_params.dram_size, */
  /*        enclave->pa_params.runtime_base, */
  /*        enclave->pa_params.user_base, */
  /*        enclave->pa_params.free_base); */
  return ENCLAVE_SUCCESS;
}

void platform_switch_to_enclave(struct enclave* enclave){
  pmp_set(scratch_rid, PMP_ALL_PERM);
  //printm("Switching to an enclave with scratchpad access\r\n");
}

void platform_switch_from_enclave(struct enclave* enclave){
  pmp_set(scratch_rid, PMP_NO_PERM);
}

uint64_t platform_random(){
#pragma message("Platform has no entropy source, this is unsafe. TEST ONLY")
    static uint64_t w = 0, s = 0xb5ad4eceda1ce2a9;

    unsigned long cycles;
    asm volatile ("rdcycle %0" : "=r" (cycles));

    // from Middle Square Weyl Sequence algorithm
    uint64_t x = cycles;
    x *= x;
    x += (w += s);
    return (x>>32) | (x<<32);
}
