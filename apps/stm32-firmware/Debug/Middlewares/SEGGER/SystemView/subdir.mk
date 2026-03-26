################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/SEGGER/SystemView/SEGGER_SYSVIEW.c 

OBJS += \
./Middlewares/SEGGER/SystemView/SEGGER_SYSVIEW.o 

C_DEPS += \
./Middlewares/SEGGER/SystemView/SEGGER_SYSVIEW.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/SEGGER/SystemView/%.o Middlewares/SEGGER/SystemView/%.su Middlewares/SEGGER/SystemView/%.cyclo: ../Middlewares/SEGGER/SystemView/%.c Middlewares/SEGGER/SystemView/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Middlewares/SEGGER/RTT -I../Middlewares/SEGGER/SystemView -I../Middlewares/SEGGER/Config -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-SEGGER-2f-SystemView

clean-Middlewares-2f-SEGGER-2f-SystemView:
	-$(RM) ./Middlewares/SEGGER/SystemView/SEGGER_SYSVIEW.cyclo ./Middlewares/SEGGER/SystemView/SEGGER_SYSVIEW.d ./Middlewares/SEGGER/SystemView/SEGGER_SYSVIEW.o ./Middlewares/SEGGER/SystemView/SEGGER_SYSVIEW.su

.PHONY: clean-Middlewares-2f-SEGGER-2f-SystemView

