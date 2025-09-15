#ifndef LPS22HH_APP_H
#define LPS22HH_APP_H
#include "lps22hh.h"
#include "lps22hh_reg.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "stdio.h"
//#include "i2c.h"

typedef enum {
    lps22hhERROR = -1, lps22hhInit = 0, lps22hhFIFOFull = 1, lps22hhFIFOReady = 2
}lps22hhStateTypeDef;

extern LPS22HH_IO_t lps22hhIO;
extern LPS22HH_Object_t lps22hhObj;
extern lps22hh_odr_t odr;
extern lps22hhStateTypeDef lps22hhState;


int32_t lps22hh_App_Init(LPS22HH_Object_t *, LPS22HH_IO_t *);
int32_t lps22hh_App_FIFO_Init(LPS22HH_Object_t *, uint8_t);
int32_t lps22hh_App_GetOutputDataRate(LPS22HH_Object_t *, float *);
int32_t lps22hh_App_SetOutputDataRate(LPS22HH_Object_t *, float);


#endif //LPS22HH_APP_H
