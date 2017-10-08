//
//  RJTRadio.m
//  AudioFrequency
//
//  Created by Robby Tong on 10/7/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RJTRadio.h"
#include "../Transceivers/TransceiverBPSK.h"
#include "../PlotController/PlotController.h"

@interface RJTRadio()
// private method declaration
@end

@implementation RJTRadio
{
    // private instance variables
    PlotController * mPlotController;
    TransceiverBPSK * mTransceiver;
}

void RJTRadio_callBack(void * obj, RadioMsg * msg)
{
    
}

// methods
-(id) initWithTxFreq:(double)txFreq rxFreq:(double)rxFreq ifBandWidth:(double)bw ifFreq:(double)ifFreq cyclesPerBit:(int)cycles
{
    self = [super init];
    if (self)
    {
        mPlotController = new PlotController();
        mTransceiver =
        new TransceiverBPSK(RJTRadio_callBack, NULL, 44.1E3, txFreq, rxFreq, ifFreq, bw, cycles, mPlotController);
    }
    return self;
}

-(id) initWithTxFreq:(double)txFreq rxFreq:(double)rxFreq
{
    return [self initWithTxFreq:txFreq rxFreq:rxFreq ifBandWidth:2E3 cyclesPerBit:10];
}

-(void) dealloc
{
    
}

-(void) sendWithByteArray:(char *)msg
{
    
}

-(void) sendWithObject:(NSString *)msg
{
    
}

-(void) start
{
    
}

-(void) stop
{
    
}
@end
