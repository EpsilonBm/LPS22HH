#include "lps22hh_app.h"

lps22hhStateTypeDef lps22hhState; // state Variable of lps22hh
LPS22HH_IO_t lps22hhIO;           // function interface of IO(pIO)
LPS22HH_Object_t lps22hhObj;      // lps22hh object(pObj)
//lps22hh_odr_t odr;                // output data rate
#ifdef __STM32F4xx_HAL_H
extern I2C_HandleTypeDef hi2c1;   // REMAIN CONFUSION(Will there be confliction), but it must be the
extern I2C_HandleTypeDef hi2c2;
#define lps22hhi2c hi2c2
#endif
lps22hhFIFODataTypeDef lps22hhFIFOData;
lps22hhFIFODebugDataTypeDef lps22hhFIFODebugData;

static void lps22hh_App_IO_Init(LPS22HH_IO_t *);
static int32_t LPS22HH_WriteReg(uint16_t, uint16_t, uint8_t *, uint16_t);
static int32_t LPS22HH_ReadReg(uint16_t, uint16_t, uint8_t *, uint16_t);
static void LPS22HH_Delay(uint32_t);
static int32_t LPS22HH_GetTick();

/**
  * @brief  The application initialization, including the init of the IO to the HAL library, the init of the whole sensor and the press and temperature sensor.
  */
int32_t lps22hh_App_Init(LPS22HH_Object_t * pObj, LPS22HH_IO_t * pIO) {
    // Init the IO acro
    lps22hh_App_IO_Init(pIO);
    // Init the Object of sensor
    LPS22HH_RegisterBusIO(pObj, &lps22hhIO);
    // Init the sensor
    if (LPS22HH_COMMON_Driver.Init(pObj) == LPS22HH_ERROR) {
        //
        return LPS22HH_ERROR;
    }else if (LPS22HH_PRESS_Driver.Enable(pObj) == LPS22HH_ERROR) {
        //
        return LPS22HH_ERROR;
    }else if (LPS22HH_TEMP_Driver.Enable(pObj) == LPS22HH_ERROR) {
        //
        return LPS22HH_ERROR;
    }else {
        lps22hh_App_SetOutputDataRate(pObj, 100);
        return LPS22HH_OK;
    }
}

int32_t lps22hh_App_FIFO_Init(LPS22HH_Object_t *pObj, uint8_t watermark) {
    // Write the 0x14U(FIFO_WTM) -> WTM as (watermark - 1) (watermark - 1)
    // Attention: The watermark level write must before the FIFO enabled!(refer an5209-9.2.1)
    if (LPS22HH_FIFO_Set_Watermark_Level(pObj, watermark - 1) != LPS22HH_OK) {
        return LPS22HH_ERROR;
    }
    // Write the 0x13U(FIFO_CTRL) -> f_mode as (1) (1)
    if (LPS22HH_FIFO_Set_Mode(pObj, LPS22HH_FIFO_MODE) != LPS22HH_OK) {
        return LPS22HH_ERROR;
    }
    // Write the 0x12U(CTRL_REG3) -> int_f_full as (1) (32)
    if (LPS22HH_FIFO_Set_Interrupt(pObj, 1) != LPS22HH_OK) {
        return LPS22HH_ERROR;
    }
    // Write the 0x13U(FIFO_CTRL) -> stop_on_wtm as (1) (1->9)
    // if (LPS22HH_FIFO_Stop_On_Watermark(pObj, 1) != LPS22HH_OK) {
    //     return LPS22HH_ERROR;
    // }
    return LPS22HH_OK;
}

int32_t lps22hh_App_GetOutputDataRate(LPS22HH_Object_t * pObj, float *Odr)
{
    int32_t ret = LPS22HH_OK;
    lps22hh_odr_t odr_low_level;

    if (lps22hh_data_rate_get(&(pObj->Ctx), &odr_low_level) != LPS22HH_OK)
    {
        return LPS22HH_ERROR;
    }

    switch (odr_low_level)
    {
        case LPS22HH_POWER_DOWN:
            *Odr = 0.0f;
            break;

        case LPS22HH_1_Hz:
            *Odr = 1.0f;
            break;

        case LPS22HH_10_Hz:
            *Odr = 10.0f;
            break;

        case LPS22HH_25_Hz:
            *Odr = 25.0f;
            break;

        case LPS22HH_50_Hz:
            *Odr = 50.0f;
            break;

        case LPS22HH_75_Hz:
            *Odr = 75.0f;
            break;

        case LPS22HH_100_Hz:
            *Odr = 100.0f;
            break;

        case LPS22HH_200_Hz:
            *Odr = 200.0f;
            break;

        default:
            ret = LPS22HH_ERROR;
            break;
    }

    return ret;
}

int32_t lps22hh_App_SetOutputDataRate(LPS22HH_Object_t * pObj, float Odr) {
    if (lps22hhObj.is_initialized == 0U) {
        return LPS22HH_ERROR;
    }else if (lps22hhObj.press_is_enabled == 0U && pObj->temp_is_enabled == 0U) {
        return LPS22HH_ERROR;
    }else if (lps22hhObj.press_is_enabled == 1U) {
        return LPS22HH_PRESS_SetOutputDataRate(pObj, Odr);
    }else if (lps22hhObj.temp_is_enabled == 1U) {
        return LPS22HH_TEMP_SetOutputDataRate(pObj, Odr);
    }
    return LPS22HH_ERROR;
}

int32_t lps22hh_App_Get_FIFO_Data(LPS22HH_Object_t * pObj, lps22hhFIFODataTypeDef * FIFOData) {
    if (LPS22HH_FIFO_Get_Level(pObj, &(FIFOData->level)) != LPS22HH_OK) {
        return LPS22HH_ERROR;
    }
    for (int i = 0;i < FIFOData->level;i++) {
        if (LPS22HH_FIFO_Get_Data(pObj, &(FIFOData->press[i]), &(FIFOData->temperature[i])) != LPS22HH_OK) {
            return LPS22HH_ERROR;
        }
    }
    return LPS22HH_OK;
}

lps22hhFIFODebugDataTypeDef * lps22hh_debug_FIFOstatus(LPS22HH_Object_t * pObj, lps22hhFIFODebugDataTypeDef * FIFODebugData) {
    lps22hh_read_reg(&(pObj->Ctx), LPS22HH_CTRL_REG3, (uint8_t *) &(FIFODebugData->CTRL_REG3), 1);
    lps22hh_read_reg(&(pObj->Ctx), LPS22HH_FIFO_CTRL, (uint8_t *) &(FIFODebugData->FIFO_CTRL), 1);
    lps22hh_read_reg(&(pObj->Ctx), LPS22HH_FIFO_WTM, (uint8_t *) &(FIFODebugData->FIFO_WTM), 1);
    lps22hh_read_reg(&(pObj->Ctx), LPS22HH_FIFO_STATUS1, (uint8_t *) &(FIFODebugData->FIFO_STATUS1), 1);
    lps22hh_read_reg(&(pObj->Ctx), LPS22HH_FIFO_STATUS2, (uint8_t *) &(FIFODebugData->FIFO_STATUS2), 1);
    lps22hh_read_reg(&(pObj->Ctx), LPS22HH_STATUS, (uint8_t *) &(FIFODebugData->STATUS), 1);
    lps22hh_read_reg(&(pObj->Ctx), LPS22HH_WHO_AM_I, (uint8_t *) &(FIFODebugData->WHO_AM_I), 1);

    return FIFODebugData;
}

/**
  * @brief  Init the IO function interface (HAL library)
  */
static void lps22hh_App_IO_Init(LPS22HH_IO_t * pIO) {
#ifdef __STM32F4xx_HAL_H
    pIO->Init = HAL_I2C_Init;           // REMAIN CONFUSION
    pIO->DeInit = HAL_I2C_DeInit;       // The function pointer is unused in lps22hh.c (The DeInit logic of the sensor is a brand-new and interface-free process, so REMAIN CONFUSION in case of being used somewhere else)
#endif
    pIO->BusType = LPS22HH_I2C_BUS;
    pIO->Address = LPS22HH_I2C_ADD_L;   // Default I2C Address 0xB9
    pIO->WriteReg = LPS22HH_WriteReg;
    pIO->ReadReg = LPS22HH_ReadReg;
    pIO->Delay = LPS22HH_Delay;
    pIO->GetTick = LPS22HH_GetTick;
}

#ifdef __STM32F4xx_HAL_H
/**
  * @brief  The application of the interface "LPS22HH_WriteReg_Func"
  */
static int32_t LPS22HH_WriteReg(uint16_t dev_addr, uint16_t reg_addr, uint8_t *data, uint16_t len) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        &lps22hhi2c,
        dev_addr,
        reg_addr,
        I2C_MEMADD_SIZE_8BIT,
        data,
        len,
        HAL_MAX_DELAY
    );
    return (status == HAL_OK) ? LPS22HH_OK : LPS22HH_ERROR;
}

/**
  * @brief  The application of the interface "LPS22HH_ReadReg_Func"
  */
static int32_t LPS22HH_ReadReg(uint16_t dev_addr, uint16_t reg_addr, uint8_t *data, uint16_t len) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        &lps22hhi2c,
        dev_addr,
        reg_addr,
        I2C_MEMADD_SIZE_8BIT,
        data,
        len,
        HAL_MAX_DELAY
    );
    return (status == HAL_OK) ? LPS22HH_OK : LPS22HH_ERROR;
}

/**
  * @brief  The application of the interface "LPS22HH_Delay_Func"
  */

static void LPS22HH_Delay(uint32_t milliseconds) {
    HAL_Delay(milliseconds);
}

/**
  * @brief  The application of the interface "LPS22HH_GetTick_Func"
  */
static int32_t LPS22HH_GetTick() {
    HAL_StatusTypeDef status =  HAL_GetTick();
    return (status == HAL_OK) ? LPS22HH_OK : LPS22HH_ERROR;
}
#endif

