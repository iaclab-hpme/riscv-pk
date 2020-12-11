#include "plugins/hpme.h"
#include "sm.h"
#include "plugins/mmio.h"
#include "enclave.h"
#include <string.h>

extern struct enclave enclaves[];

static uint64_t genDMAReqReg(uint8_t wen, uint64_t length, uint64_t addr,
        uint64_t dataType){
  addr = addr << 2;
  length = (length >> 2) << 50;
  dataType = dataType << 60;
  return (wen + length + addr + dataType);
}


static void write_isenc(uint8_t is_enc){
  reg_write8(HPME_ISENC_ADDR, is_enc);
}

static void write_key(enclave_id id){
  reg_write64(HPME_DATAINLO_ADDR, enclaves[id].ped.key[0]);
  reg_write64(HPME_DATAINHI_ADDR, enclaves[id].ped.key[1]);
  reg_write8(HPME_DATAINTYPE_ADDR, DMA_REQ_DATA_TYPE_KEY);
}

static void write_counter(uint64_t counterHi, uint64_t counterLo){
  reg_write64(HPME_DATAINLO_ADDR, counterLo);
  reg_write64(HPME_DATAINHI_ADDR, counterHi);
  reg_write8(HPME_DATAINTYPE_ADDR, DMA_REQ_DATA_TYPE_CNT);
}

static void write_dma_read_request(uint64_t addr_src){
  uint64_t dmaReqReg = genDMAReqReg(1, 4096, addr_src, 0);
  reg_write64(HPME_DMAREQREG_ADDR, dmaReqReg);
}

static void write_dma_write_request(uint64_t addr_dst){
  uint64_t dmaReqReg = genDMAReqReg(2, 4096, addr_dst, 0);
  reg_write64(HPME_DMAREQREG_ADDR, dmaReqReg);
}

static void read_mac(uint64_t mac[2]){
  uint8_t dataOutType = 0;
  while(dataOutType != DATA_OUT_TYPE_MAC){
      dataOutType = reg_read8(HPME_DATAOUTTYPE_ADDR);
  }
  mac[0] = reg_read64(HPME_DATAOUTLO_ADDR);
  mac[1] = reg_read64(HPME_DATAOUTHI_ADDR);
  reg_write8(HPME_DATAOUTRCVD_ADDR, 1);
  reg_write8(HPME_DATAOUTRCVD_ADDR, 0);
  // reg_read8(HPME_DUMMY_ADDR);
}

static void wait_for_data_in_rcvd(){
  uint8_t status = 0;
  uint8_t i = 0;
  while(!status){
      status = reg_read8(HPME_DATAINSTATUS_ADDR);
  }
  reg_write8(HPME_DATAINTYPE_ADDR, 0);
}

static void wait_for_dma_done(){
  uint8_t status = 0;
  while(!status){
      status = reg_read8(HPME_DMASTATUS_ADDR);
  }
  reg_write64(HPME_DMAREQREG_ADDR, 0);
}

uintptr_t do_sbi_hpme(enclave_id id, uintptr_t call_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3){
  printm("[sm] do_sbi_hpme: %d, %d, 0x%lx, 0x%lx, 0x%lx, 0x%lx\r\n", id, call_id, arg0, arg1, arg2, arg3);
  switch(call_id){
    case HPME_CALLID_ENC_SWAP:
      memcpy((void*)HPME_SWAP_BUFFER_ADDR, (void*)arg1, 4096);
    case HPME_CALLID_ENC:
      write_isenc(1);
      write_key(id);
      wait_for_data_in_rcvd();
      write_counter(0, arg2);
      wait_for_data_in_rcvd();
      write_dma_read_request(arg0);
      wait_for_dma_done();
      read_mac((uint64_t*)arg3);
      write_dma_write_request(arg1);
      wait_for_dma_done();
      break;
    case HPME_CALLID_DEC:
      write_isenc(0);
      write_key(id);
      wait_for_data_in_rcvd();
      write_counter(0, arg1);
      wait_for_data_in_rcvd();
      write_dma_read_request(HPME_SWAP_BUFFER_ADDR);
      wait_for_dma_done();
      read_mac((uint64_t*)arg2);
      write_dma_write_request(arg0);
      wait_for_dma_done();
      break;
  }
  return 0;
}