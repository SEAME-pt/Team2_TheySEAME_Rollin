################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Components/ism330dhcx/ism330dhcx_reg.c 

OBJS += \
./Drivers/Components/ism330dhcx/ism330dhcx_reg.o 

C_DEPS += \
./Drivers/Components/ism330dhcx/ism330dhcx_reg.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Components/ism330dhcx/%.o Drivers/Components/ism330dhcx/%.su Drivers/Components/ism330dhcx/%.cyclo: ../Drivers/Components/ism330dhcx/%.c Drivers/Components/ism330dhcx/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Middlewares/SEGGER/RTT -I../Middlewares/SEGGER/SystemView -I../Middlewares/SEGGER/Config -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../Drivers/Components/ism330dhcx -I../Drivers/BSP/B-U585I-IOT02A -I../Drivers/Components/Common -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Components-2f-ism330dhcx

clean-Drivers-2f-Components-2f-ism330dhcx:
	-$(RM) ./Drivers/Components/ism330dhcx/ism330dhcx_reg.cyclo ./Drivers/Components/ism330dhcx/ism330dhcx_reg.d ./Drivers/Components/ism330dhcx/ism330dhcx_reg.o ./Drivers/Components/ism330dhcx/ism330dhcx_reg.su

.PHONY: clean-Drivers-2f-Components-2f-ism330dhcx

