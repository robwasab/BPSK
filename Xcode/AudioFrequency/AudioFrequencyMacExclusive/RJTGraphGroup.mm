//
//  RJTGraphGroup.m
//  CoreGraphicsMac
//
//  Created by Robby Tong on 10/5/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RJTGraphGroup.h"
#import "RJTGraph.h"
#include "DataSource.h"
#include "SineWave.hpp"
#include <vector>
#import "AudioFrequencyWrapper.h"
using namespace std;

extern vector<DataSource *> dataSources;

@interface RJTGraphGroup()
{
    //MARK: Private Properties
    
}
// MARK: Private Methods
-(void) layoutGraphs;
-(void) graphsToCreate:(int)num;
-(void) checkDataSources:(NSTimer *)timer;

@end

@implementation RJTGraphGroup
{
    //MARK: Private Instance Variables
    int mNumGraphs;
    CGFloat mWidth;
    CGFloat mHeight;
    BOOL mExpandRemainingGraphs;
    NSMutableArray * mGraphs;
    NSTimer * mTimer;
}

-(id) initWithNumGraphs:(int)numGraphs
                  width:(CGFloat)width
                 height:(CGFloat)height
  expandRemainingGraphs:(BOOL)expand
{
    self = [super init];
    if (self != nil)
    {
        mWidth = width;
        mHeight = height;
        mExpandRemainingGraphs = expand;
        mGraphs = [[NSMutableArray alloc] init];
        [self graphsToCreate:numGraphs];
        mTimer = [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(checkDataSources:) userInfo:nil repeats:YES];
    }
    return self;
}

-(id) init
{
    return nil;
}

-(void) graphsToCreate:(int)numGraphs
{
    mNumGraphs = numGraphs;
    
    [mGraphs removeAllObjects];
    
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

-(void) checkDataSources:(NSTimer *)timer
{
    if (dataSources.size() == mNumGraphs)
    {
        [mTimer invalidate];
        mTimer = nil;
        [self loadAudioFrequencyDataSources];
    }
}

-(void) loadAudioFrequencyDataSources
{
    for (int k = 0; k < mNumGraphs; k++)
    {
        printf("%s\n", dataSources[k]->name());
        [[mGraphs objectAtIndex:k] setDataSource:dataSources[k]];
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
    int numColumns;
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
    NSInteger count = [mGraphs count];
    for (int k = 0; k < count; k++)
    {
        [[mGraphs objectAtIndex:k] draw:ctx];
    }
}

@end
