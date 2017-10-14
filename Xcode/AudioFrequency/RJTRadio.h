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
@required
-(void) receivedData:(uint8_t *)data dataLen:(uint8_t)len;
-(void) finishedStopping;
@end


@interface RJTRadio:NSObject
{
    @public
    uint8_t mReceiveDataCount;
    uint8_t mReceiveDataLen;
    uint8_t mReceiveData[256];
    void * mDataSourcesRef;
}

@property (weak, atomic) id<RJTRadioReceiveProtocol> mReceiveDelegate;

// Public methods
-(id) initWithTxFreq:(double)txFreq rxFreq:(double)rxFreq ifBandWidth:(double)bw ifFreq:(double)ifFreq cyclesPerBit:(int)cycles;
-(id) initWithTxFreq:(double)txFreq rxFreq:(double)rxFreq;
-(void) dealloc;
-(void) sendWithByteArray:(const uint8_t *)inMsg length:(uint8_t)len;
-(void) sendWithObject:(NSString *)msg;
-(void) start;
-(void) stop;
@end

#endif /* Radio_h */
