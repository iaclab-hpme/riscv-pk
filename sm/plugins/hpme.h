#ifndef SM_HPME_H
#define SM_HPME_H

#include "plugins/plugins.h"
#include "enclave.h"

#define HPME_DMAREQREG_ADDR       0x78000000
#define HPME_DMASTATUS_ADDR       0x78000008
#define HPME_DATAINLO_ADDR        0x78000010
#define HPME_DATAINHI_ADDR        0x78000018
#define HPME_DATAINTYPE_ADDR      0x78000020
#define HPME_DATAINSTATUS_ADDR    0x78000028
#define HPME_ISENC_ADDR           0x78000029
#define HPME_DATAOUTLO_ADDR       0x78000030
#define HPME_DATAOUTHI_ADDR       0x78000038
#define HPME_DATAOUTTYPE_ADDR     0x78000040
#define HPME_DATAOUTRCVD_ADDR     0x78000048

#define HPME_SWAP_BUFFER_ADDR     0x703FF000

#define HPME_CALLID_ENC           0x01
#define HPME_CALLID_DEC           0x02
#define HPME_CALLID_ENC_SWAP      0x03

#define DMA_REQ_DATA_TYPE_DATA    0
#define DMA_REQ_DATA_TYPE_KEY     1
#define DMA_REQ_DATA_TYPE_CNT     2
#define DMA_REQ_DATA_TYPE_MAC     3

#define DATA_OUT_TYPE_MAC         1

uintptr_t do_sbi_hpme(enclave_id id, uintptr_t call_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);


#endif