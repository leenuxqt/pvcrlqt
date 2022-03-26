#ifndef RTU_MASTER_H
#define RTU_MASTER_H

#include <stdint.h>

#ifdef __linux
#include <modbus.h>

extern uint16_t *tab_rp_registers;

int initRtuMaster(int portno);

void CallData();

void releaseRtuMaster();
#endif

#endif 
