################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Sensors/battery.c \
../Core/Src/Sensors/sensors_processor.c \
../Core/Src/Sensors/sensors_queue.c \
../Core/Src/Sensors/speed.c 

OBJS += \
./Core/Src/Sensors/battery.o \
./Core/Src/Sensors/sensors_processor.o \
./Core/Src/Sensors/sensors_queue.o \
./Core/Src/Sensors/speed.o 

C_DEPS += \
./Core/Src/Sensors/battery.d \
./Core/Src/Sensors/sensors_processor.d \
./Core/Src/Sensors/sensors_queue.d \
./Core/Src/Sensors/speed.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Sensors/%.o Core/Src/Sensors/%.su Core/Src/Sensors/%.cyclo: ../Core/Src/Sensors/%.c Core/Src/Sensors/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Sensors

clean-Core-2f-Src-2f-Sensors:
	-$(RM) ./Core/Src/Sensors/battery.cyclo ./Core/Src/Sensors/battery.d ./Core/Src/Sensors/battery.o ./Core/Src/Sensors/battery.su ./Core/Src/Sensors/sensors_processor.cyclo ./Core/Src/Sensors/sensors_processor.d ./Core/Src/Sensors/sensors_processor.o ./Core/Src/Sensors/sensors_processor.su ./Core/Src/Sensors/sensors_queue.cyclo ./Core/Src/Sensors/sensors_queue.d ./Core/Src/Sensors/sensors_queue.o ./Core/Src/Sensors/sensors_queue.su ./Core/Src/Sensors/speed.cyclo ./Core/Src/Sensors/speed.d ./Core/Src/Sensors/speed.o ./Core/Src/Sensors/speed.su

.PHONY: clean-Core-2f-Src-2f-Sensors

