################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Communication/communication.c \
../Core/Src/Communication/mcp2515.c 

OBJS += \
./Core/Src/Communication/communication.o \
./Core/Src/Communication/mcp2515.o 

C_DEPS += \
./Core/Src/Communication/communication.d \
./Core/Src/Communication/mcp2515.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Communication/%.o Core/Src/Communication/%.su Core/Src/Communication/%.cyclo: ../Core/Src/Communication/%.c Core/Src/Communication/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Communication

clean-Core-2f-Src-2f-Communication:
	-$(RM) ./Core/Src/Communication/communication.cyclo ./Core/Src/Communication/communication.d ./Core/Src/Communication/communication.o ./Core/Src/Communication/communication.su ./Core/Src/Communication/mcp2515.cyclo ./Core/Src/Communication/mcp2515.d ./Core/Src/Communication/mcp2515.o ./Core/Src/Communication/mcp2515.su

.PHONY: clean-Core-2f-Src-2f-Communication

