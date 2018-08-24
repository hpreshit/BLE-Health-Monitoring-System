################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.1/platform/emdrv/ustimer/src/ustimer.c 

OBJS += \
./platform/emdrv/ustimer/src/ustimer.o 

C_DEPS += \
./platform/emdrv/ustimer/src/ustimer.d 


# Each subdirectory must supply rules for building sources it contributes
platform/emdrv/ustimer/src/ustimer.o: C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.1/platform/emdrv/ustimer/src/ustimer.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-DHAL_CONFIG=1' '-D__HEAP_SIZE=0xD00' '-D__STACK_SIZE=0x800' '-DEFR32BG1B232F256GM56=1' -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\Device\SiliconLabs\EFR32BG1B\Source\GCC" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\emdrv\uartdrv\inc" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\halconfig\inc\hal-config" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\bootloader\api" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\hardware\kit\EFR32BG1_BRD4302A\config" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\hardware\kit\common\drivers" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\radio\rail_lib\common" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\emdrv\tempdrv\src" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\emdrv\sleep\inc" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\protocol\bluetooth_2.7\ble_stack\inc\soc" -I"C:/C:\SiliconLabs\SimplicityStudio\v4\developer\toolchains\gnu_arm\4.9_2015q3\/lib/gcc/arm-none-eabi/4.9.3/include" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\emdrv\common\inc" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\protocol\bluetooth_2.7\ble_stack\inc\common" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\src" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\app\bluetooth_2.7\common\stack_bridge" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\emdrv\sleep\src" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\emlib\src" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\Device\SiliconLabs\EFR32BG1B\Source" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\radio\rail_lib\chip\efr32" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\emlib\inc" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\bootloader" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\CMSIS\Include" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\emdrv\tempdrv\inc" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\hardware\kit\common\bsp" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\platform\Device\SiliconLabs\EFR32BG1B\Include" -I"C:\Users\Preshit\SimplicityStudio\v4_workspace\Particle_Sensor_\hardware\kit\common\halconfig" -I"C:\SiliconLabs\SimplicityStudio\v4\developer\sdks\gecko_sdk_suite\v2.1\platform\emdrv\ustimer\inc" -I"C:\SiliconLabs\SimplicityStudio\v4\developer\sdks\gecko_sdk_suite\v2.1\platform\emdrv\ustimer\config" -O0 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"platform/emdrv/ustimer/src/ustimer.d" -MT"platform/emdrv/ustimer/src/ustimer.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


