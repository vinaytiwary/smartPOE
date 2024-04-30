################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./tm4c1233e6pz_startup_ccs.obj" \
"./Sources/ADC_Core.obj" \
"./Sources/Clock.obj" \
"./Sources/DS3231.obj" \
"./Sources/Display.obj" \
"./Sources/E2P.obj" \
"./Sources/IO_cntrl.obj" \
"./Sources/PC_Cmds.obj" \
"./Sources/SPI.obj" \
"./Sources/SysTick_Timer.obj" \
"./Sources/UartCore.obj" \
"./Sources/_common.obj" \
"./Sources/delay.obj" \
"./Sources/flashCore.obj" \
"./Sources/flash_logger.obj" \
"./Sources/gprs.obj" \
"./Sources/gps.obj" \
"./Sources/main.obj" \
"../tm4c1233e6pz.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \
-l"C:/ti/TivaWare_C_Series-2.2.0.295/driverlib/ccs/Debug/driverlib.lib" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Sources/subdir_vars.mk
-include subdir_rules.mk
-include Sources/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Telecom_IoT_v2_00.out \

EXE_OUTPUTS__QUOTED += \
"Telecom_IoT_v2_00.out" \

BIN_OUTPUTS += \
Telecom_IoT_v2_00.hex \

BIN_OUTPUTS__QUOTED += \
"Telecom_IoT_v2_00.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "Telecom_IoT_v2_00.out"

# Tool invocations
Telecom_IoT_v2_00.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --opt_for_speed=3 --define=ccs="ccs" --define=PART_TM4C1233E6PZ -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"Telecom_IoT_v2_00.map" --heap_size=0 --stack_size=512 -i"C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/lib" -i"C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="Telecom_IoT_v2_00_linkInfo.xml" --rom_model -o "Telecom_IoT_v2_00.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

Telecom_IoT_v2_00.hex: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: Arm Hex Utility'
	"C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/bin/armhex" --diag_wrap=off -o "Telecom_IoT_v2_00.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "tm4c1233e6pz_startup_ccs.obj" "Sources\ADC_Core.obj" "Sources\Clock.obj" "Sources\DS3231.obj" "Sources\Display.obj" "Sources\E2P.obj" "Sources\IO_cntrl.obj" "Sources\PC_Cmds.obj" "Sources\SPI.obj" "Sources\SysTick_Timer.obj" "Sources\UartCore.obj" "Sources\_common.obj" "Sources\delay.obj" "Sources\flashCore.obj" "Sources\flash_logger.obj" "Sources\gprs.obj" "Sources\gps.obj" "Sources\main.obj" 
	-$(RM) "tm4c1233e6pz_startup_ccs.d" 
	-$(RM) "Sources\ADC_Core.d" "Sources\Clock.d" "Sources\DS3231.d" "Sources\Display.d" "Sources\E2P.d" "Sources\IO_cntrl.d" "Sources\PC_Cmds.d" "Sources\SPI.d" "Sources\SysTick_Timer.d" "Sources\UartCore.d" "Sources\_common.d" "Sources\delay.d" "Sources\flashCore.d" "Sources\flash_logger.d" "Sources\gprs.d" "Sources\gps.d" "Sources\main.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

