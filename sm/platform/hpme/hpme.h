//
// Created by yintianyu on 2020/10/13.
//

#ifndef KEYSTONE_HPME_H
#define KEYSTONE_HPME_H
#include "pmp.h"

/* PMP Region ID for the scratchpad */
region_id scratch_rid;

// L2 Zero Device (Scratchpad) info
#define L2_SCRATCH_START (0x70000000)
#define L2_SCRATCH_STOP  (0x70400000)

/* 4 MB */
#define L2_SIZE (4*1024*1024)

// No special data needed for default platform
struct platform_enclave_data{

};


#endif //KEYSTONE_HPME_H
