################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
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
all: nivel

dependents:
	-cd /home/utnso/workspacePruebasTP/sharedNivel/Debug && $(MAKE) clean all

# Tool invocations
nivel: $(OBJS) $(USER_OBJS) /home/utnso/workspacePruebasTP/sharedNivel/Debug/libsharedNivel.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspacePruebasTP/sharedNivel/Debug" -o "nivel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) nivel
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspacePruebasTP/sharedNivel/Debug/libsharedNivel.so:

-include ../makefile.targets
