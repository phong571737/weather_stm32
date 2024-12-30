################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/\ W25Qxx.c \
../Core/Src/ILI9341_STM32_Driver.c \
../Core/Src/Image_Lib.c \
../Core/Src/calibrate.c \
../Core/Src/fonts.c \
../Core/Src/main.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/touch.c \
../Core/Src/z_flash_W25QXXX.c 

OBJS += \
./Core/Src/\ W25Qxx.o \
./Core/Src/ILI9341_STM32_Driver.o \
./Core/Src/Image_Lib.o \
./Core/Src/calibrate.o \
./Core/Src/fonts.o \
./Core/Src/main.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/touch.o \
./Core/Src/z_flash_W25QXXX.o 

C_DEPS += \
./Core/Src/\ W25Qxx.d \
./Core/Src/ILI9341_STM32_Driver.d \
./Core/Src/Image_Lib.d \
./Core/Src/calibrate.d \
./Core/Src/fonts.d \
./Core/Src/main.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/touch.d \
./Core/Src/z_flash_W25QXXX.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/\ W25Qxx.o: ../Core/Src/\ W25Qxx.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -IC:/Users/DELL/STM32Cube/Repository/STM32Cube_FW_F1_V1.8.6/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -IC:/Users/DELL/STM32Cube/Repository/STM32Cube_FW_F1_V1.8.6/Drivers/STM32F1xx_HAL_Driver/Inc -IC:/Users/DELL/STM32Cube/Repository/STM32Cube_FW_F1_V1.8.6/Drivers/CMSIS/Device/ST/STM32F1xx/Include -IC:/Users/DELL/STM32Cube/Repository/STM32Cube_FW_F1_V1.8.6/Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Core/Src/ W25Qxx.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -IC:/Users/DELL/STM32Cube/Repository/STM32Cube_FW_F1_V1.8.6/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -IC:/Users/DELL/STM32Cube/Repository/STM32Cube_FW_F1_V1.8.6/Drivers/STM32F1xx_HAL_Driver/Inc -IC:/Users/DELL/STM32Cube/Repository/STM32Cube_FW_F1_V1.8.6/Drivers/CMSIS/Device/ST/STM32F1xx/Include -IC:/Users/DELL/STM32Cube/Repository/STM32Cube_FW_F1_V1.8.6/Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/\ W25Qxx.cyclo ./Core/Src/\ W25Qxx.d ./Core/Src/\ W25Qxx.o ./Core/Src/\ W25Qxx.su ./Core/Src/ILI9341_STM32_Driver.cyclo ./Core/Src/ILI9341_STM32_Driver.d ./Core/Src/ILI9341_STM32_Driver.o ./Core/Src/ILI9341_STM32_Driver.su ./Core/Src/Image_Lib.cyclo ./Core/Src/Image_Lib.d ./Core/Src/Image_Lib.o ./Core/Src/Image_Lib.su ./Core/Src/calibrate.cyclo ./Core/Src/calibrate.d ./Core/Src/calibrate.o ./Core/Src/calibrate.su ./Core/Src/fonts.cyclo ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f1xx_hal_msp.cyclo ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_it.cyclo ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f1xx.cyclo ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su ./Core/Src/touch.cyclo ./Core/Src/touch.d ./Core/Src/touch.o ./Core/Src/touch.su ./Core/Src/z_flash_W25QXXX.cyclo ./Core/Src/z_flash_W25QXXX.d ./Core/Src/z_flash_W25QXXX.o ./Core/Src/z_flash_W25QXXX.su

.PHONY: clean-Core-2f-Src

