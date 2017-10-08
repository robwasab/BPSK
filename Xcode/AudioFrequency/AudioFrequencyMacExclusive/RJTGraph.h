//
//  RJTGraph.h
//  CoreGraphicsMac
//
//  Created by Robby Tong on 10/2/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#ifndef RJTGraph_h
#define RJTGraph_h

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import "RJTGraphGroup.h"

typedef void * DataSourceRef;

@interface RJTGraph : NSObject

@property (weak, atomic) id <RJTViewUpdateDelegate> delegate;

//init(x:CGFloat, y:CGFloat, width:CGFloat, height:CGFloat)
-(id) initWithOriginAtX:(CGFloat)x
              originAtY:(CGFloat)y
                  width:(CGFloat)w
                 height:(CGFloat)h;

-(id) init;
-(void) dealloc;
-(void) useDemoSineWaveAsDataSource;
-(void) setDemoSineWavePeriod:(int)period;
-(void) setDataSource:(DataSourceRef)dataSource;
-(void) setDimensionWidth:(CGFloat)width dimensionHeight:(CGFloat)height;
-(void) setOriginAtX:(CGFloat)x originAtY:(CGFloat)y;
-(void) setTitle:(NSString *)title;
-(void) draw:(CGContextRef) ctx;

@end

#endif /* RJTGraph_h */
