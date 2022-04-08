#ifndef MYMODBUSFUNC_H
#define MYMODBUSFUNC_H

#include <stdint.h>

typedef unsigned short int uint16;
typedef unsigned long int uint32;


/* Get a float from 4 bytes (Modbus) without any conversion (ABCD) */
float my_modbus_get_float_abcd(const uint16_t *src);

/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float my_modbus_get_float_cdab(const uint16_t *src);

/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float my_modbus_get_float_dcba(const uint16_t *src);

/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float my_modbus_get_float_badc(const uint16_t *src);

#endif // MYMODBUSFUNC_H
