################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Tests/build/vendor/cmock/src/cmock.c 

OBJS += \
./Core/Tests/build/vendor/cmock/src/cmock.o 

C_DEPS += \
./Core/Tests/build/vendor/cmock/src/cmock.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Tests/build/vendor/cmock/src/%.o Core/Tests/build/vendor/cmock/src/%.su Core/Tests/build/vendor/cmock/src/%.cyclo: ../Core/Tests/build/vendor/cmock/src/%.c Core/Tests/build/vendor/cmock/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Tests-2f-build-2f-vendor-2f-cmock-2f-src

clean-Core-2f-Tests-2f-build-2f-vendor-2f-cmock-2f-src:
	-$(RM) ./Core/Tests/build/vendor/cmock/src/cmock.cyclo ./Core/Tests/build/vendor/cmock/src/cmock.d ./Core/Tests/build/vendor/cmock/src/cmock.o ./Core/Tests/build/vendor/cmock/src/cmock.su

.PHONY: clean-Core-2f-Tests-2f-build-2f-vendor-2f-cmock-2f-src

