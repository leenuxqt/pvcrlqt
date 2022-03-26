/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "rtumaster.h"

/* The goal of this program is to check all major functions of
   libmodbus:
   - write_coil
   - read_bits
   - write_coils
   - write_register
   - read_registers
   - write_registers
   - read_registers

   All these functions are called with random values on a address
   range defined by the following defines.
*/
#define LOOP             1
#define SUDIAN_RTU_ID    3
#define ADDRESS_START   0x00
#define ADDRESS_END     0x34

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */

modbus_t *ctx;
int rc;
int nb_fail;
int nb_loop;
int addr;
int nb;

uint16_t *tab_rp_registers;

int initRtuMaster(int portno)
{

    return -1;//test

    /* RTU */
    char portstr[32] = {0};
    sprintf(portstr,"/dev/ttyS%d", portno);
    ctx = modbus_new_rtu(portstr, 9600, 'N', 8, 1);
    modbus_set_slave(ctx, SUDIAN_RTU_ID);

    modbus_set_debug(ctx, TRUE);


    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the different memory spaces */
    nb = ADDRESS_END - ADDRESS_START;

    tab_rp_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

    return 0;
}

void CallData()
{
    nb_loop = nb_fail = 0;
    while (nb_loop++ < LOOP) {
        /* R/W MULTIPLE REGISTERS */            
        rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
        if (rc != nb) {
            printf("ERROR modbus_read_registers (%d)\n", rc);
            printf("Address = %d, nb = %d\n", addr, nb);
            nb_fail++;
        } else {
		    int i=0;
            // for (i=0; i<nb; i++) {
            //     printf("Address = %d, value %d (0x%X)\n", addr, tab_rp_registers[i], tab_rp_registers[i]);                            
            // }

            for (i=0; i<nb/2; i++) {
                printf("Address = %d, value %f (0x%X)\n", i*2, modbus_get_float_dcba(&tab_rp_registers[i*2]), tab_rp_registers[i*2]);                            
            }            
        }   
        printf("Test: ");
    }
}

void releaseRtuMaster()
{
    /* Free the memory */
    free(tab_rp_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);
}
