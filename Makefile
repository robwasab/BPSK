CC:=g++
INCLUDE:=-I /usr/local/include 
FLAGS:=-g 
LIBRARY:=-L /usr/local/lib
OPTIONS:=-lm -lfftw3f -lpthread -lportaudio

OUTPUT:=main

# QT Stuff
PLOT_OBJECTS=PlotController.o circularbuffer.o mainwindow.o moc_mainwindow.o moc_plot.o plot.o

# Prepend PLOT_OBJECTS with PlotController/
PLOT_PATHS=$(addprefix PlotController/,$(PLOT_OBJECTS))

# Copied these from PlotController/Makefile
ifdef PLOT_PATHS
LIBS= -F/usr/local/Cellar/qt5/5.7.0/lib -L /usr/local/opt/qwt/lib/qwt.framework/ /usr/local/opt/qwt/lib/qwt.framework/qwt -framework QtPrintSupport -framework QtWidgets -framework QtGui -framework QtCore -framework DiskArbitration -framework IOKit -framework QtOpenGL -framework OpenGL -framework AGL 
PLOT_CLEAN=make -C ./PlotController clean
#QT_ENABLE=-D QT_ENABLE
endif

Module_objects=Module.o
Module_paths=$(addprefix Module/,$(Module_objects))

generator_objects=generator.o
generator_paths=$(addprefix MaximumLength/,$(generator_objects))

TaskScheduler_objects=TaskScheduler.o
TaskScheduler_paths=$(addprefix TaskScheduler/,$(TaskScheduler_objects))

Memory_objects=Memory.o
Memory_paths=$(addprefix Memory/,$(Memory_objects))

EncoderBPSK_objects=StdinSource.o Prefix.o BPSK.o
EncoderBPSK_paths=$(addprefix EncoderBPSK/,$(EncoderBPSK_objects))

EncoderQPSK_objects=QPSK_StdinSource.o QPSK_Prefix.o QPSK_Encode.o
EncoderQPSK_paths=$(addprefix EncoderQPSK/,$(EncoderQPSK_objects))

EncoderPSK8_objects=PSK8_SigGen.o
EncoderPSK8_paths=$(addprefix EncoderPSK8/,$(EncoderPSK8_objects))

Filter_objects=BandPass.o FirFilter.o math.o FirLowPass.o PlottableFirFilter.o
Filter_paths=$(addprefix Filter/,$(Filter_objects))

CostasLoop_objects=CostasLoop.o Biquad_LowPass.o CostasLoopBlock.o Plottable_CostasLoop.o
CostasLoop_paths=$(addprefix CostasLoop/,$(CostasLoop_objects))

QPSK_objects=QPSK.o QPSKBlock.o
QPSK_paths=$(addprefix CostasLoop/,$(QPSK_objects))

WavSink_objects=WavSink.o
WavSink_paths=$(addprefix WavSink/,$(WavSink_objects))

DecoderBPSK_objects=BPSKDecoder.o
DecoderBPSK_paths=$(addprefix DecoderBPSK/,$(DecoderBPSK_objects))

Modulator_objects=Modulator.o
Modulator_paths=$(addprefix Modulator/,$(Modulator_objects))

SpectrumAnalyzer_objects=SpectrumAnalyzer.o
SpectrumAnalyzer_paths=$(addprefix SpectrumAnalyzer/,$(SpectrumAnalyzer_objects))

Constellation_objects=Constellation.o
Constellation_paths=$(addprefix Constellation/,$(Constellation_objects))

Autogain_objects=Autogain.o PlottableAutogain.o
Autogain_paths=$(addprefix Autogain/,$(Autogain_objects))

PortAudio_objects=PortAudioSimulator.o PortAudioStdin.o
PortAudio_paths=$(addprefix PortAudio/,$(PortAudio_objects))

PortAudioDriver_objects=PortAudioDriver.o
PortAudioDriver_paths=$(addprefix PortAudioDriver/,$(PortAudioDriver_objects))

Channel_objects=Channel.o
Channel_paths=$(addprefix Channel/,$(Channel_objects))

Transceivers_objects=TransceiverQPSK.o Transceiver.o TransceiverPSK8.o TransceiverBPSK.o
Transceivers_paths=$(addprefix Transceivers/,$(Transceivers_objects))

TestFramework_objects=TestFramework.o
TestFramework_paths=$(addprefix TestFramework/,$(TestFramework_objects))

Tests_objects=simple.o
Tests_paths=$(addprefix Tests/,$(Tests_objects))

RadioMsg_objects=RadioMsg.o
RadioMsg_paths=$(addprefix RadioMsg/,$(RadioMsg_objects))

CRC_objects=crc.o
CRC_paths=$(addprefix CRC-16/,$(CRC_objects))

# Default target
all: qpsk

debug:
	echo $(EncoderBPSK_paths)
	echo $(EncoderQPSK_paths)
	echo $(Tests_paths)

qpsk: qpsk.o $(Module_paths) $(EncoderBPSK_paths) $(EncoderQPSK_paths) $(EncoderPSK8_paths) $(Memory_paths) $(PLOT_PATHS) $(Filter_paths) $(CostasLoop_paths) $(QPSK_paths) $(WavSink_paths) $(DecoderBPSK_paths) $(generator_paths) $(Modulator_paths) $(SpectrumAnalyzer_paths) $(Constellation_paths) $(Autogain_paths) $(Transceivers_paths) $(SignaledThread_paths) $(TestFramework_paths) $(Tests_paths) $(RadioMsg_paths) $(Channel_paths) $(PortAudioDriver_paths) $(CRC_paths) Colors/Colors.h PlotSink/PlotSink.h switches.h SuppressPrint/SuppressPrint.h
	$(CC) $(LIBRARY) $(OPTIONS) qpsk.o $(Module_paths) $(EncoderBPSK_paths) $(EncoderQPSK_paths) $(EncoderPSK8_paths) $(Memory_paths) $(Transmitter_paths) $(QPSK_Transmitter_paths) $(Filter_paths) $(CostasLoop_paths) $(QPSK_paths) $(WavSink_paths) $(PLOT_PATHS) $(Receiver_paths) $(DecoderBPSK_paths) $(generator_paths) $(Modulator_paths) $(SpectrumAnalyzer_paths) $(Constellation_paths) $(Autogain_paths) $(Transceivers_paths) $(TestFramework_paths) $(Tests_paths) $(RadioMsg_paths) $(Channel_paths) $(PortAudioDriver_paths) $(CRC_paths) -o $(OUTPUT) $(LIBS)

qpsk.o: qpsk.cpp $(Transceivers_paths) $(TestFramework_paths) Colors/Colors.h PlotSink/PlotSink.h switches.h
	$(CC) $(QT_ENABLE) -Wall $(FLAGS) $(INCLUDE) -c qpsk.cpp

$(Module_paths):%.o:%.cpp %.h Module/Module.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(PLOT_PATHS):%.o:PlotController/plot.h PlotController/plot.cpp switches.h Module/Module.h
	make -C ./PlotController $(notdir $@)

$(TaskScheduler_paths):%.o:%.cpp %.h Queue/Queue.h Colors/Colors.h switches.h Module/Module.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Memory_paths):%.o:%.cpp %.h Memory/Block.h Colors/Colors.h switches.h Module/Module.h Queue/Queue.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(EncoderBPSK_paths):%.o: %.cpp %.h Colors/Colors.h PlotController/DataSource.h switches.h Module/Module.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(EncoderQPSK_paths):%.o: %.cpp %.h Colors/Colors.h switches.h Module/Module.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(EncoderPSK8_paths):%.o: %.cpp %.h Colors/Colors.h switches.h Module/Module.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Filter_paths):%.o: %.cpp %.h Module/Module.h switches.h Module/Module.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(CostasLoop_paths):%.o: %.cpp %.h CostasLoop/Integrator.h CostasLoop/LockDetector.h CostasLoop/RC_LowPass.h switches.h Module/Module.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(QPSK_paths):%.o: %.cpp %.h CostasLoop/CostasLoop.h CostasLoop/CostasLoopBlock.h CostasLoop/Integrator.h CostasLoop/LockDetector.h CostasLoop/RC_LowPass.h switches.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(WavSink_paths):%.o: %.cpp %.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(DecoderBPSK_paths):%.o: %.cpp %.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(generator_paths):%.o: %.cpp %.h switches.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(CRC_paths):%.o: %.cpp %.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Modulator_paths):%.o: %.cpp %.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(SpectrumAnalyzer_paths):%.o: %.cpp %.h PlotController/DataSource.h Module/Module.h switches.h Queue/Queue.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Constellation_paths):%.o: %.cpp %.h PlotController/DataSource.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Autogain_paths):%.o: %.cpp %.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(PortAudioDriver_paths):%.o: %.cpp %.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Channel_paths):%.o: %.cpp %.h Module/Module.h RadioMsg/RadioMsg.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Transceivers_paths):%.o: %.cpp %.h Module/Module.h switches.h Transceivers/Transceiver.h Signaledthread/Signaledthread.h Queue/Queue.h CRC-16/crc.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(TestFramework_paths):%.o: %.cpp %.h switches.h Stack/Stack.h Transceivers/Transceiver.h Transceivers/TransceiverQPSK.h Notify/Notify.h SignaledThread/SignaledThread.h Module/Module.h Queue/Queue.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(Tests_paths):%.o: %.cpp %.h TestFramework/TestFramework.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

$(RadioMsg_paths):%.o: %.cpp %.h Module/Module.h switches.h
	$(CC) -Wall $(FLAGS) $(INCLUDE) -c $< -o $@

clean:
#	rm $(TaskScheduler_paths)
	rm $(Module_paths)
	rm $(Memory_paths)
	rm $(EncoderBPSK_paths)
	rm $(EncoderQPSK_paths)
	rm $(EncoderPSK8_paths)
	rm $(Filter_paths)
	rm $(CostasLoop_paths)
	rm $(QPSK_paths)
	rm $(WavSink_paths)
	rm $(DecoderBPSK_paths)
	rm $(generator_paths)
	rm $(Modulator_paths)
	rm $(SpectrumAnalyzer_paths)
	rm $(Constellation_paths)
	rm $(Autogain_paths)
	rm $(PortAudioDriver_paths)
	rm $(Channel_paths)
	rm $(Transceivers_paths)
	rm $(TestFramework_paths)
	rm $(Tests_paths)
	rm $(RadioMsg_paths)
	rm $(CRC_paths)
	$(PLOT_CLEAN)

