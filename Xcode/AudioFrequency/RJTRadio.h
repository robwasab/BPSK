//
//  Radio.h
//  AudioFrequency
//
//  Created by Robby Tong on 10/7/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#ifndef Radio_h
#define Radio_h

@protocol RJTRadioReceiveProtocol<NSObject>
-(void) receivedData:(char *)data;
@end


@interface RJTRadio:NSObject

@property (weak, atomic) id<RJTRadioReceiveProtocol> mReceiveDelegate;

// Public methods
-(id) initWithTxFreq:(double)txFreq rxFreq:(double)rxFreq ifBandWidth:(double)bw ifFreq:(double)ifFreq cyclesPerBit:(int)cycles;
-(id) initWithTxFreq:(double)txFreq rxFreq:(double)rxFreq;
-(void) dealloc;

-(void) sendWithByteArray:(char *)msg;
-(void) sendWithObject:(NSString *)msg;
-(void) start;
-(void) stop;
@end

#endif /* Radio_h */
