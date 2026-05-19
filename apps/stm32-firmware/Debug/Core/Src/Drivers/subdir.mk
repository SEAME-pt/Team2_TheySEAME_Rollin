################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Drivers/ina219.c \
../Core/Src/Drivers/pca9685.c 

OBJS += \
./Core/Src/Drivers/ina219.o \
./Core/Src/Drivers/pca9685.o 

C_DEPS += \
./Core/Src/Drivers/ina219.d \
./Core/Src/Drivers/pca9685.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Drivers/%.o Core/Src/Drivers/%.su Core/Src/Drivers/%.cyclo: ../Core/Src/Drivers/%.c Core/Src/Drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Middlewares/SEGGER/RTT -I../Middlewares/SEGGER/SystemView -I../Middlewares/SEGGER/Config -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../Drivers/BSP/B-U585I-IOT02A -I../Drivers/BSP/Components/iis2mdc -I../Drivers/BSP/Components/ism330dhcx -I/Drivers/BSP/Components/Common -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Drivers

clean-Core-2f-Src-2f-Drivers:
	-$(RM) ./Core/Src/Drivers/ina219.cyclo ./Core/Src/Drivers/ina219.d ./Core/Src/Drivers/ina219.o ./Core/Src/Drivers/ina219.su ./Core/Src/Drivers/pca9685.cyclo ./Core/Src/Drivers/pca9685.d ./Core/Src/Drivers/pca9685.o ./Core/Src/Drivers/pca9685.su

.PHONY: clean-Core-2f-Src-2f-Drivers

