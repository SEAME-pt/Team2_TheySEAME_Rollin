################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/SEGGER/Config/SEGGER_SYSVIEW_Config_ThreadX.c \
../Middlewares/SEGGER/Config/SEGGER_SYSVIEW_ThreadX.c 

OBJS += \
./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_Config_ThreadX.o \
./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_ThreadX.o 

C_DEPS += \
./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_Config_ThreadX.d \
./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_ThreadX.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/SEGGER/Config/%.o Middlewares/SEGGER/Config/%.su Middlewares/SEGGER/Config/%.cyclo: ../Middlewares/SEGGER/Config/%.c Middlewares/SEGGER/Config/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Middlewares/SEGGER/RTT -I../Middlewares/SEGGER/SystemView -I../Middlewares/SEGGER/Config -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../Drivers/BSP/B-U585I-IOT02A -I../Drivers/BSP/Components/iis2mdc -I../Drivers/BSP/Components/ism330dhcx -I../Drivers/BSP/Components/Common -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-SEGGER-2f-Config

clean-Middlewares-2f-SEGGER-2f-Config:
	-$(RM) ./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_Config_ThreadX.cyclo ./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_Config_ThreadX.d ./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_Config_ThreadX.o ./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_Config_ThreadX.su ./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_ThreadX.cyclo ./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_ThreadX.d ./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_ThreadX.o ./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_ThreadX.su

.PHONY: clean-Middlewares-2f-SEGGER-2f-Config

