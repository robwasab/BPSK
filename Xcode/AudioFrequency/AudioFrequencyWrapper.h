//
//  AudioFrequencyWrapper.h
//  AudioFrequency
//
//  Created by Robby Tong on 9/29/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#ifndef AudioFrequencyWrapper_h
#define AudioFrequencyWrapper_h

#import <Foundation/Foundation.h>

@interface AudioFrequencyWrapper:NSObject

// Public properties
@property BOOL block;

// Public methods
-(id) initWithBlockOnStart:(BOOL)blockOnStart;
-(id) initWithArgc:(int)argc withArgv:(char**)argv shouldBlockOnStart:(BOOL)blockOnStart;
-(void) dealloc;
-(void) mainLoop;
-(void) start;
-(void) stop;
@end

#endif /* AudioFrequencyWrapper_h */
