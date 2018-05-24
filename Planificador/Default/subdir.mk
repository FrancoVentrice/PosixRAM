################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ConfigPlanificador.c \
../ConsolaPlanificador.c \
../Planificador.c 

OBJS += \
./ConfigPlanificador.o \
./ConsolaPlanificador.o \
./Planificador.o 

C_DEPS += \
./ConfigPlanificador.d \
./ConsolaPlanificador.d \
./Planificador.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2018-1c-PosixRAM/shared" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


