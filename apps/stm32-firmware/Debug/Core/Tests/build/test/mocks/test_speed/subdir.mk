################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Tests/build/test/mocks/test_speed/mock_tx_api.c 

OBJS += \
./Core/Tests/build/test/mocks/test_speed/mock_tx_api.o 

C_DEPS += \
./Core/Tests/build/test/mocks/test_speed/mock_tx_api.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Tests/build/test/mocks/test_speed/%.o Core/Tests/build/test/mocks/test_speed/%.su Core/Tests/build/test/mocks/test_speed/%.cyclo: ../Core/Tests/build/test/mocks/test_speed/%.c Core/Tests/build/test/mocks/test_speed/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Tests-2f-build-2f-test-2f-mocks-2f-test_speed

clean-Core-2f-Tests-2f-build-2f-test-2f-mocks-2f-test_speed:
	-$(RM) ./Core/Tests/build/test/mocks/test_speed/mock_tx_api.cyclo ./Core/Tests/build/test/mocks/test_speed/mock_tx_api.d ./Core/Tests/build/test/mocks/test_speed/mock_tx_api.o ./Core/Tests/build/test/mocks/test_speed/mock_tx_api.su

.PHONY: clean-Core-2f-Tests-2f-build-2f-test-2f-mocks-2f-test_speed

