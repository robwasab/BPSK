//
//  RJTGraphGroup.m
//  CoreGraphicsMac
//
//  Created by Robby Tong on 10/5/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>
#import "RJTGraphGroup.h"
#import "RJTGraph.h"
#import "RJTRadio.h"
#include "DataSource.h"
#include "SineWave.hpp"
#include <vector>

using namespace std;

@interface RJTGraphGroup()
{
    //MARK: Private Properties
    
}
// MARK: Private Methods
-(void) layoutGraphs;
-(void) graphsToCreate:(size_t)num;
-(void) removeAllGraphs;
-(void) loadDataSourcesFromRJTRadio:(RJTRadio *)radio;

@end

@implementation RJTGraphGroup
{
    //MARK: Private Instance Variables
    size_t mNumGraphs;
    CGFloat mWidth;
    CGFloat mHeight;
    CGFloat mX;
    CGFloat mY;
    BOOL mExpandRemainingGraphs;
    NSMutableArray * mGraphs;
}

-(id) initWithNumGraphs:(int)numGraphs
                  frame:(CGRect)frame
  expandRemainingGraphs:(BOOL)expand
{
    self = [super init];
    if (self != nil)
    {
        mWidth = frame.size.width;
        mHeight = frame.size.height;
        mX = frame.origin.x;
        mY = frame.origin.y;
        
        mExpandRemainingGraphs = expand;
        mGraphs = [[NSMutableArray alloc] init];
        [self graphsToCreate:numGraphs];
    }
    return self;
}

-(id) init
{
    return nil;
}

-(void) removeAllGraphs
{
    for (int k = 0; k < mNumGraphs; k++)
    {
        RJTGraph * graph = [mGraphs objectAtIndex:k];
        [graph stopTimer];
    }
    
    [mGraphs removeAllObjects];
    mNumGraphs = 0;
}

-(void) graphsToCreate:(size_t)numGraphs
{
    [self removeAllGraphs];
    
    mNumGraphs = numGraphs;
    
    for (int k = 0; k < mNumGraphs; k++)
    {
        RJTGraph * graph;
        graph = [[RJTGraph alloc] init];
        [graph setDemoSineWavePeriod:k+1];
        [mGraphs addObject:graph];
        graph.delegate = self;
    }
    [self layoutGraphs];
}

-(void) dealloc
{
}

-(void) redraw
{
    [_delegate redraw];
}

-(void) loadDataSourcesFromRJTRadio:(RJTRadio *)radio
{
    vector<DataSource *> * sources = (vector<DataSource *> *) radio->mDataSourcesRef;
    
    if (mNumGraphs != sources->size())
    {
        [self removeAllGraphs];
        [self graphsToCreate:sources->size()];
    }
    
    for (int k = 0; k < sources->size(); k++)
    {
        [[mGraphs objectAtIndex:k] setDataSource:(*sources)[k]];
    }
}

-(void) graphsShouldExpand:(BOOL)expandGraphs
{
    mExpandRemainingGraphs = expandGraphs;
    [self layoutGraphs];
}

-(void) setDimensionsWithRect:(CGSize)size
{
    mWidth = size.width;
    mHeight = size.height;
    [self layoutGraphs];
}

-(void) setDimensionsWithWidth:(CGFloat)width
                    withHeight:(CGFloat)height
{
    mWidth = width;
    mHeight = height;
    [self layoutGraphs];
}

-(void) layoutGraphs
{
    size_t numColumns;
    CGFloat columnWidth;
    
    numColumns = mNumGraphs / 4 + (mNumGraphs % 4 > 0 ? 1 : 0);
    columnWidth = mWidth / numColumns;
    
    for (int k = 0; k < mNumGraphs; k++)
    {
        CGFloat graphWidth = columnWidth;
        CGFloat graphHeight = mHeight / 4.0;
        
        CGFloat graphX = (k / 4) * graphWidth;
        CGFloat graphY = (k % 4) * graphHeight;
        
        if (mExpandRemainingGraphs == YES)
        {
            int numberOfGraphsInLastColumn = mNumGraphs % 4;
            if (k >= mNumGraphs - numberOfGraphsInLastColumn)
            {
                graphHeight = mHeight / numberOfGraphsInLastColumn;
                graphY = graphHeight * (k % 4);
            }
        }
        
        
        [[mGraphs objectAtIndex:k] setDimensionWidth:graphWidth dimensionHeight:graphHeight];
        
        [[mGraphs objectAtIndex:k] setOriginAtX:graphX originAtY:graphY];
    }
}

-(NSArray *) getGraphs
{
    return mGraphs;
}

-(void) draw:(CGContextRef)ctx
{
    CGRect rect = CGRectMake(mX, mY, mWidth, mHeight);
    CGContextSetStrokeColorWithColor(ctx, UIColor.blueColor.CGColor);
    
    CGContextStrokeRect(ctx, rect);
    
    CGContextTranslateCTM(ctx, 0, mHeight);
    CGContextScaleCTM(ctx, 1.0, -1.0);
    NSInteger count = [mGraphs count];
    for (int k = 0; k < count; k++)
    {
        [[mGraphs objectAtIndex:k] draw:ctx];
    }
}

@end
