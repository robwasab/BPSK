CC:=g++
INCLUDE:=-I /usr/local/include 
LIBRARY:=-L /usr/local/lib
OPTIONS:=-lm -g

OUTPUT:=main

# QT Stuff
PLOT_OBJECTS=PlotController.o circularbuffer.o mainwindow.o moc_mainwindow.o moc_plot.o plot.o

# Prepend PLOT_OBJECTS with PlotController/
PLOT_PATHS=$(addprefix PlotController/,$(PLOT_OBJECTS))

# Copied these from PlotController/Makefile
ifdef PLOT_PATHS
LIBS= -F/usr/local/Cellar/qt5/5.7.0/lib -L /usr/local/opt/qwt/lib/qwt.framework/ /usr/local/opt/qwt/lib/qwt.framework/qwt -framework QtPrintSupport -framework QtWidgets -framework QtGui -framework QtCore -framework DiskArbitration -framework IOKit -framework QtOpenGL -framework OpenGL -framework AGL 
PLOT_CLEAN=make -C ./PlotController clean
QT_ENABLE=-D QT_ENABLE
endif

generator_objects=generator.o
generator_paths=$(addprefix MaximumLength/,$(generator_objects))

TaskScheduler_objects=TaskScheduler.o
TaskScheduler_paths=$(addprefix TaskScheduler/,$(TaskScheduler_objects))

Memory_objects=Memory.o
Memory_paths=$(addprefix Memory/,$(Memory_objects))

Transmitter_objects=StdinSource.o Prefix.o Pulseshape.o BPSK.o
Transmitter_paths=$(addprefix Transmitter/,$(Transmitter_objects))

Filter_objects=BandPass.o
Filter_paths=$(addprefix Filter/,$(Filter_objects))

CostasLoop_objects=CostasLoop.o Biquad_LowPass.o CostasLoopBlock.o
CostasLoop_paths=$(addprefix CostasLoop/,$(CostasLoop_objects))

WavSink_objects=WavSink.o
WavSink_paths=$(addprefix WavSink/,$(WavSink_objects))

Receiver_objects=BPSKDecoder.o
Receiver_paths=$(addprefix Receiver/,$(Receiver_objects))

# Default target
all: main

main: main.o $(TaskScheduler_paths) $(Memory_paths) $(Transmitter_paths) $(PLOT_PATHS) $(Filter_paths) $(CostasLoop_paths) $(WavSink_paths) $(Receiver_paths) $(generator_paths) Colors/Colors.h PlotSink/PlotSink.h
	$(CC) $(LIBRARY) $(OPTIONS) main.o $(TaskScheduler_paths) $(Memory_paths) $(Transmitter_paths) $(Filter_paths) $(CostasLoop_paths) $(WavSink_paths) $(PLOT_PATHS) $(Receiver_paths) $(generator_paths) -o $(OUTPUT) $(LIBS)

main.o: main.cpp $(TaskScheduler_paths) $(Memory_paths) $(Transmitter_paths) $(Plot_PATHS) Colors/Colors.h
	$(CC) $(QT_ENABLE) -Wall $(INCLUDE) -c main.cpp

$(PLOT_PATHS):%.o:PlotController/plot.h PlotController/plot.cpp
	make -C ./PlotController $(notdir $@)

$(TaskScheduler_paths):%.o:%.cpp %.h Queue/Queue.h Colors/Colors.h
	$(CC) -Wall $(INCLUDE) -c $< -o $@

$(Memory_paths):%.o:%.cpp %.h Memory/Block.h Colors/Colors.h
	$(CC) -Wall $(INCLUDE) -c $< -o $@

$(Transmitter_paths):%.o: %.cpp %.h Colors/Colors.h PlotController/DataSource.h
	$(CC) -Wall $(INCLUDE) -c $< -o $@

$(Filter_paths):%.o: %.cpp %.h Module/Module.h
	$(CC) -Wall $(INCLUDE) -c $< -o $@

$(CostasLoop_paths):%.o: %.cpp %.h CostasLoop/Integrator.h CostasLoop/LockDetector.h CostasLoop/RC_LowPass.h
	$(CC) -Wall $(INCLUDE) -c $< -o $@

$(WavSink_paths):%.o: %.cpp %.h Module/Module.h
	$(CC) -Wall $(INCLUDE) -c $< -o $@

$(Receiver_paths):%.o: %.cpp %.h Module/Module.h
	$(CC) -Wall $(INCLUDE) -c $< -o $@

$(generator_paths):%.o: %.cpp %.h
	$(CC) -Wall $(INCLUDE) -c $< -o $@

clean:
	rm $(TaskScheduler_paths)
	rm $(Memory_paths)
	rm $(Transmitter_paths)
	rm $(Filter_paths)
	rm $(CostasLoop_paths)
	rm $(WavSink_paths)
	rm $(Receiver_paths)
	rm $(generator_paths)
	$(PLOT_CLEAN)

