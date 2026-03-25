/**
  ******************************************************************************
  * @file    bsp_bus_wrapper.c
  * @brief   Simple BSP bus wrapper implementation for I2C functions
  *          This file provides the missing BSP I2C functions required by
  *          motion sensor drivers.
  ******************************************************************************
  */

#include "main.h"
#include "i2c.h"
#include "b_u585i_iot02a_errno.h"

/* Private define ------------------------------------------------------------*/
#define BSP_I2C_TIMEOUT_MAX    1000U

/* External variables --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

/**
  * @brief  Initializes I2C2 interface
  * @retval BSP status
  */
int32_t BSP_I2C2_Init(void) {
    /* I2C2 is already initialized by MX_I2C2_Init() in main */
    return BSP_ERROR_NONE;
}

/**
  * @brief  DeInitializes I2C2 interface  
  * @retval BSP status
  */
int32_t BSP_I2C2_DeInit(void) {
    if (HAL_I2C_DeInit(&hi2c2) != HAL_OK) {
        return BSP_ERROR_BUS_FAILURE;
    }
    return BSP_ERROR_NONE;
}

/**
  * @brief  Write register through BUS IO bus
  * @param  DevAddr   Device address on BUS Bus.
  * @param  Reg       The target register address to write
  * @param  pData     The target register value to be written 
  * @param  Length    data length in bytes
  * @retval BSP status
  */
int32_t BSP_I2C2_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    int32_t ret = BSP_ERROR_BUS_FAILURE;
    
    if (HAL_I2C_Mem_Write(&hi2c2, DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, 
                         pData, Length, BSP_I2C_TIMEOUT_MAX) == HAL_OK) {
        ret = BSP_ERROR_NONE;
    }
    
    return ret;
}

/**
  * @brief  Read register through BUS IO bus
  * @param  DevAddr   Device address on BUS Bus.
  * @param  Reg       The target register address to read
  * @param  pData     The target register value to be read
  * @param  Length    data length in bytes
  * @retval BSP status
  */
int32_t BSP_I2C2_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    int32_t ret = BSP_ERROR_BUS_FAILURE;
    
    if (HAL_I2C_Mem_Read(&hi2c2, DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, 
                        pData, Length, BSP_I2C_TIMEOUT_MAX) == HAL_OK) {
        ret = BSP_ERROR_NONE;
    }
    
    return ret;
}

/**
  * @brief  Get system tick in milliseconds
  * @retval Current tick value in ms
  */
int32_t BSP_GetTick(void) {
    return HAL_GetTick();
}

/**
  * @brief  Initializes I2C1 interface (if needed)
  * @retval BSP status
  */
int32_t BSP_I2C1_Init(void) {
    /* I2C1 is already initialized by MX_I2C1_Init() in main */
    return BSP_ERROR_NONE;
}

/**
  * @brief  DeInitializes I2C1 interface  
  * @retval BSP status
  */
int32_t BSP_I2C1_DeInit(void) {
    if (HAL_I2C_DeInit(&hi2c1) != HAL_OK) {
        return BSP_ERROR_BUS_FAILURE;
    }
    return BSP_ERROR_NONE;
}

/**
  * @brief  Write register through I2C1 bus
  * @param  DevAddr   Device address on I2C1 Bus.
  * @param  Reg       The target register address to write
  * @param  pData     The target register value to be written 
  * @param  Length    data length in bytes
  * @retval BSP status
  */
int32_t BSP_I2C1_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    int32_t ret = BSP_ERROR_BUS_FAILURE;
    
    if (HAL_I2C_Mem_Write(&hi2c1, DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, 
                         pData, Length, BSP_I2C_TIMEOUT_MAX) == HAL_OK) {
        ret = BSP_ERROR_NONE;
    }
    
    return ret;
}

/**
  * @brief  Read register through I2C1 bus
  * @param  DevAddr   Device address on I2C1 Bus.
  * @param  Reg       The target register address to read
  * @param  pData     The target register value to be read
  * @param  Length    data length in bytes
  * @retval BSP status
  */
int32_t BSP_I2C1_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    int32_t ret = BSP_ERROR_BUS_FAILURE;
    
    if (HAL_I2C_Mem_Read(&hi2c1, DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, 
                        pData, Length, BSP_I2C_TIMEOUT_MAX) == HAL_OK) {
        ret = BSP_ERROR_NONE;
    }
    
    return ret;
}