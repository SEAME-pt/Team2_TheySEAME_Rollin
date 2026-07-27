################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.c \
../Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_usbpd_pwr.c 


OBJS += \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_usbpd_pwr.o 


C_DEPS += \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_usbpd_pwr.d 



# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/B-U585I-IOT02A/%.o Drivers/BSP/B-U585I-IOT02A/%.su Drivers/BSP/B-U585I-IOT02A/%.cyclo: ../Drivers/BSP/B-U585I-IOT02A/%.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32U585xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../Middlewares/SEGGER/RTT -I../Middlewares/SEGGER/SystemView -I../Middlewares/SEGGER/Config -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-B-2d-U585I-2d-IOT02A

clean-Drivers-2f-BSP-2f-B-2d-U585I-2d-IOT02A:
	-$(RM) ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_usbpd_pwr.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_usbpd_pwr.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_usbpd_pwr.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_usbpd_pwr.su
	-$(RM) ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.su

.PHONY: clean-Drivers-2f-BSP-2f-B-2d-U585I-2d-IOT02A

