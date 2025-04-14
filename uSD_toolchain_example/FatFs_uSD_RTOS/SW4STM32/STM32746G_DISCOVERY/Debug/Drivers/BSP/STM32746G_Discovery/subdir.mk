################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/pierremirouze/Documents/GitHub/Kartrack_SW/uSD_toolchain_example/FatFs_uSD_RTOS/Drivers/BSP/STM32746G-Discovery/stm32746g_discovery.c \
C:/Users/pierremirouze/Documents/GitHub/Kartrack_SW/uSD_toolchain_example/FatFs_uSD_RTOS/Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_sd.c 

OBJS += \
./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery.o \
./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery_sd.o 

C_DEPS += \
./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery.d \
./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery_sd.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32746G_Discovery/stm32746g_discovery.o: C:/Users/pierremirouze/Documents/GitHub/Kartrack_SW/uSD_toolchain_example/FatFs_uSD_RTOS/Drivers/BSP/STM32746G-Discovery/stm32746g_discovery.c Drivers/BSP/STM32746G_Discovery/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F746xx -DUSE_STM32746G_DISCOVERY -c -I../../../Inc -I../../../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../../../Drivers/STM32F7xx_HAL_Driver/Inc -I../../../Drivers/BSP/STM32746G-Discovery -I../../../Middlewares/Third_Party/FatFs/src -I../../../Middlewares/Third_Party/FatFs/src/drivers -I../../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/STM32746G_Discovery/stm32746g_discovery_sd.o: C:/Users/pierremirouze/Documents/GitHub/Kartrack_SW/uSD_toolchain_example/FatFs_uSD_RTOS/Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_sd.c Drivers/BSP/STM32746G_Discovery/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F746xx -DUSE_STM32746G_DISCOVERY -c -I../../../Inc -I../../../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../../../Drivers/STM32F7xx_HAL_Driver/Inc -I../../../Drivers/BSP/STM32746G-Discovery -I../../../Middlewares/Third_Party/FatFs/src -I../../../Middlewares/Third_Party/FatFs/src/drivers -I../../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM32746G_Discovery

clean-Drivers-2f-BSP-2f-STM32746G_Discovery:
	-$(RM) ./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery.cyclo ./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery.d ./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery.o ./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery.su ./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery_sd.cyclo ./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery_sd.d ./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery_sd.o ./Drivers/BSP/STM32746G_Discovery/stm32746g_discovery_sd.su

.PHONY: clean-Drivers-2f-BSP-2f-STM32746G_Discovery

