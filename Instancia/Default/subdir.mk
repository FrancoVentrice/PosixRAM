################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ComunicacionesInstancia.c \
../ConfigInstancia.c \
../CoreInstancia.c \
../DeprecatedInstancia.c \
../EntradasInstancia.c \
../Instancia.c \
../PantallaInstancia.c \
../algoReemplazoInstancia.c 

OBJS += \
./ComunicacionesInstancia.o \
./ConfigInstancia.o \
./CoreInstancia.o \
./DeprecatedInstancia.o \
./EntradasInstancia.o \
./Instancia.o \
./PantallaInstancia.o \
./algoReemplazoInstancia.o 

C_DEPS += \
./ComunicacionesInstancia.d \
./ConfigInstancia.d \
./CoreInstancia.d \
./DeprecatedInstancia.d \
./EntradasInstancia.d \
./Instancia.d \
./PantallaInstancia.d \
./algoReemplazoInstancia.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2018-1c-PosixRAM/shared" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


