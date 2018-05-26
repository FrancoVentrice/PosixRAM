################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	$(MAKE) --no-print-directory pre-build
	$(MAKE) --no-print-directory main-build

# Main-build Target
main-build: Planificador

dependents:
	-cd /home/utnso/tp-2018-1c-PosixRAM/shared/Default && $(MAKE) clean all

# Tool invocations
Planificador: $(OBJS) $(USER_OBJS) /home/utnso/tp-2018-1c-PosixRAM/shared/Default/libshared.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L/home/utnso/tp-2018-1c-PosixRAM/shared/Default -o "Planificador" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Planificador
	-@echo ' '

pre-build:
	-rm -rf logs
	-@echo ' '

.PHONY: all clean dependents main-build pre-build
/home/utnso/tp-2018-1c-PosixRAM/shared/Default/libshared.so:

-include ../makefile.targets
