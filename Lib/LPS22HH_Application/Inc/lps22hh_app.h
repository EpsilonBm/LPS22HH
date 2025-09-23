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

typedef struct {
    uint8_t CTRL_REG3;
    uint8_t FIFO_CTRL;
    uint8_t FIFO_WTM;
    uint8_t FIFO_STATUS1;
    uint8_t FIFO_STATUS2;
    uint8_t STATUS;
    uint8_t WHO_AM_I;
}lps22hhFIFODebugDataTypeDef;

typedef struct {
    uint8_t level;
    float press[128];
    float temperature[128];
}lps22hhFIFODataTypeDef;

extern LPS22HH_IO_t lps22hhIO;
extern LPS22HH_Object_t lps22hhObj;
extern lps22hh_odr_t odr;
extern lps22hhStateTypeDef lps22hhState;
extern lps22hhFIFODataTypeDef lps22hhFIFOData;
extern lps22hhFIFODebugDataTypeDef lps22hhFIFODebugData;


int32_t lps22hh_App_Init(LPS22HH_Object_t *, LPS22HH_IO_t *);
int32_t lps22hh_App_FIFO_Init(LPS22HH_Object_t *, uint8_t);
int32_t lps22hh_App_GetOutputDataRate(LPS22HH_Object_t *, float *);
int32_t lps22hh_App_SetOutputDataRate(LPS22HH_Object_t *, float);
int32_t lps22hh_App_Get_FIFO_Data(LPS22HH_Object_t *, lps22hhFIFODataTypeDef *);
lps22hhFIFODebugDataTypeDef * lps22hh_debug_FIFOstatus(LPS22HH_Object_t *, lps22hhFIFODebugDataTypeDef *);


#endif //LPS22HH_APP_H
