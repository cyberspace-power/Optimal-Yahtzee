################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Database.cpp \
../src/Yahtzee.cpp \
../src/YahtzeeGame.cpp \
../src/test.cpp 

OBJS += \
./src/Database.o \
./src/Yahtzee.o \
./src/YahtzeeGame.o \
./src/test.o 

CPP_DEPS += \
./src/Database.d \
./src/Yahtzee.d \
./src/YahtzeeGame.d \
./src/test.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -std=c++17 -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


