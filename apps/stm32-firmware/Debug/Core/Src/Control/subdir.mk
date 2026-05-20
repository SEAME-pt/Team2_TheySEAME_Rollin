################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Control/PID.c \
../Core/Src/Control/control_queue.c \
../Core/Src/Control/steering.c 

OBJS += \
./Core/Src/Control/PID.o \
./Core/Src/Control/control_queue.o \
./Core/Src/Control/steering.o 

C_DEPS += \
./Core/Src/Control/PID.d \
./Core/Src/Control/control_queue.d \
./Core/Src/Control/steering.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Control/%.o Core/Src/Control/%.su Core/Src/Control/%.cyclo: ../Core/Src/Control/%.c Core/Src/Control/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Middlewares/SEGGER/RTT -I../Middlewares/SEGGER/SystemView -I../Middlewares/SEGGER/Config -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../Drivers/BSP/B-U585I-IOT02A -I../Drivers/BSP/Components/iis2mdc -I../Drivers/BSP/Components/ism330dhcx -I../Drivers/BSP/Components/Common -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Control

clean-Core-2f-Src-2f-Control:
	-$(RM) ./Core/Src/Control/PID.cyclo ./Core/Src/Control/PID.d ./Core/Src/Control/PID.o ./Core/Src/Control/PID.su ./Core/Src/Control/control_queue.cyclo ./Core/Src/Control/control_queue.d ./Core/Src/Control/control_queue.o ./Core/Src/Control/control_queue.su ./Core/Src/Control/steering.cyclo ./Core/Src/Control/steering.d ./Core/Src/Control/steering.o ./Core/Src/Control/steering.su

.PHONY: clean-Core-2f-Src-2f-Control

