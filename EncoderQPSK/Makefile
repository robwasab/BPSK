CC:=g++
INCLUDE:=-I /usr/local/include 
FLAGS:=-g
LIBRARY:=-L /usr/local/lib
OPTIONS:=-lm -lfftw3f -lpthread -lportaudio

OUTPUT:=test

TaskScheduler_objects=TaskScheduler.o
TaskScheduler_paths=$(addprefix ../TaskScheduler/,$(TaskScheduler_objects))

Memory_objects=Memory.o
Memory_paths=$(addprefix ../Memory/,$(Memory_objects))

MaximumLength_objects=generator.o
MaximumLength_paths=$(addprefix ../MaximumLength/,$(MaximumLength_objects))

BPSK_objects=BPSK.o 
BPSK_paths=$(addprefix ../Transmitter/,$(BPSK_objects))

Transmitter_objects=QPSK_StdinSource.o QPSK_Prefix.o QPSK_Encode.o
Transmitter_paths=$(addprefix ./,$(Transmitter_objects))

# Default target
all: main

main: main.o $(TaskScheduler_paths) $(Memory_paths) $(Transmitter_paths) $(MaximumLength_paths) $(BPSK_paths) ../Colors/Colors.h 
	$(CC) $(LIBRARY) $(OPTIONS) main.o $(TaskScheduler_paths) $(Memory_paths) $(Transmitter_paths) $(MaximumLength_paths) $(BPSK_paths) -o $(OUTPUT) $(LIBS)

$(TaskScheduler_paths):%.o:%.cpp %.h ../Queue/Queue.h ../Colors/Colors.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Memory_paths):%.o:%.cpp %.h ../Memory/Block.h ../Colors/Colors.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Transmitter_paths):%.o: %.cpp %.h ../Colors/Colors.h 
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(BPSK_paths):%.o: %.cpp %.h ../Colors/Colors.h 
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(MaximumLength_paths):%.o: %.cpp %.h 
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@
