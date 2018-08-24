################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../glib/dmd/display/dmd_display.c 

OBJS += \
./glib/dmd/display/dmd_display.o 

C_DEPS += \
./glib/dmd/display/dmd_display.d 


# Each subdirectory must supply rules for building sources it contributes
glib/dmd/display/dmd_display.o: ../glib/dmd/display/dmd_display.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-D__HEAP_SIZE=0xD00' '-DHAL_CONFIG=1' '-D__STACK_SIZE=0x800' '-DEFR32BG1B232F256GM56=1' -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\radio\rail_lib\chip\efr32" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\hardware\kit\common\halconfig" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\hardware\kit\common\bsp" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\Device\SiliconLabs\EFR32BG1B\Include" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\CMSIS\Include" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\hardware\kit\common\drivers" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\bootloader\api" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\emlib\src" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\emlib\inc" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\emdrv\uartdrv\inc" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\emdrv\tempdrv\src" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\halconfig\inc\hal-config" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\emdrv\sleep\inc" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\radio\rail_lib\common" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\protocol\bluetooth_2.7\ble_stack\inc\common" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\emdrv\common\inc" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\app\bluetooth_2.7\common\stack_bridge" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\emdrv\tempdrv\inc" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\protocol\bluetooth_2.7\ble_stack\inc\soc" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\emdrv\sleep\src" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\hardware\kit\EFR32BG1_BRD4302A\config" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\Device\SiliconLabs\EFR32BG1B\Source\GCC" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\Device\SiliconLabs\EFR32BG1B\Source" -I"C:\Users\ashis\SimplicityStudio\v4_workspace\HW5\platform\bootloader" -I"C:/C:\SiliconLabs\SimplicityStudio\v4\developer\toolchains\gnu_arm\4.9_2015q3\/lib/gcc/arm-none-eabi/4.9.3/include" -O0 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"glib/dmd/display/dmd_display.d" -MT"glib/dmd/display/dmd_display.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


