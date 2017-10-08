//
//  RJTGraphGroup.h
//  CoreGraphicsMac
//
//  Created by Robby Tong on 10/5/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#ifndef RJTGraphGroup_h
#define RJTGraphGroup_h

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

@protocol RJTViewUpdateDelegate <NSObject>

-(void) redraw;

@end

@interface RJTGraphGroup : NSObject <RJTViewUpdateDelegate>
{
}

@property (weak, atomic) id <RJTViewUpdateDelegate> delegate;

-(id) initWithNumGraphs:(int)numGraphs
                  width:(CGFloat)width
                 height:(CGFloat)height
  expandRemainingGraphs:(BOOL)expand;


-(id) init;

-(void) dealloc;

-(void) redraw;

-(void) loadAudioFrequencyDataSources;

-(void) graphsShouldExpand:(BOOL)expandGraphs;

-(void) setDimensionsWithRect:(CGSize)size;

-(void) setDimensionsWithWidth:(CGFloat)width
                    withHeight:(CGFloat)height;
-(void) layoutGraphs;

-(NSArray *) getGraphs;

-(void) draw:(CGContextRef)ctx;

@end

#endif /* RJTGraphGroup_h */
