################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Tests/test/test_mcp2515.c \
../Core/Tests/test/test_speed.c 

OBJS += \
./Core/Tests/test/test_mcp2515.o \
./Core/Tests/test/test_speed.o 

C_DEPS += \
./Core/Tests/test/test_mcp2515.d \
./Core/Tests/test/test_speed.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Tests/test/%.o Core/Tests/test/%.su Core/Tests/test/%.cyclo: ../Core/Tests/test/%.c Core/Tests/test/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Tests-2f-test

clean-Core-2f-Tests-2f-test:
	-$(RM) ./Core/Tests/test/test_mcp2515.cyclo ./Core/Tests/test/test_mcp2515.d ./Core/Tests/test/test_mcp2515.o ./Core/Tests/test/test_mcp2515.su ./Core/Tests/test/test_speed.cyclo ./Core/Tests/test/test_speed.d ./Core/Tests/test/test_speed.o ./Core/Tests/test/test_speed.su

.PHONY: clean-Core-2f-Tests-2f-test

