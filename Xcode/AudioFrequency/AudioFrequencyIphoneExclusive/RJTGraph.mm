//
//  RJTGraph.m
//  AudioFrequency
//
//  Created by Robby Tong on 10/22/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#import <Foundation/Foundation.h>
//
//  Graph.m
//  CoreGraphicsMac
//
//  Created by Robby Tong on 10/2/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>

#import "RJTGraph.h"
#include "DataSource.h"
#include "SineWave.hpp"
#include <vector>

using namespace std;

@interface RJTGraph()

// MARK: Private Methods

-(void) drawTitle:(CGContextRef) ctx;
-(void) drawBoundingBox:(CGContextRef) ctx;
-(void) drawPlotAtOrigin:(CGContextRef) ctx;

-(void) drawAxis:(CGContextRef) ctx
     plotCenterX:(CGFloat) plotCenterX
     plotCenterY:(CGFloat) plotCenterY
          xScale:(CGFloat) xScale
          yScale:(CGFloat) yScale;

-(void) drawAxisTickLabels:(CGContextRef) ctx
                    xScale:(CGFloat) xScale
                    yScale:(CGFloat) yScale
      translateByXToCenter:(CGFloat) xTranslation
      translateByYToCenter:(CGFloat) yTranslation;

-(void) drawAxisTicks:(CGContextRef) ctx
               xScale:(CGFloat) xScale
               yScale:(CGFloat) yScale;

-(void) drawPlot:(CGContextRef) ctx;
-(void) updateTitleBounds;
-(void) startTimer;
-(void) timeoutOccured:(NSTimer*)timer;

@end

@implementation RJTGraph
{
    CGFloat mX;
    CGFloat mY;
    CGFloat mWidth;
    CGFloat mHeight;
    CGFloat mXAxisMargin;
    CGFloat mYAxisMargin;
    CGFloat mVerticalTickSpacing;
    CGFloat mHorizontalTickSpacing;
    CGFloat mTickLength;
    CGFloat mTitleYFraction;
    CGRect mTitleBounds;
    UIFont * mTitleFont;
    NSString * mTitle;
    NSDictionary * mAttributes;
    NSDictionary * mTickLabelFontAttributes;
    NSTimer * mTimer;
    DataSource * mDataSource;
    SineWave * mSineWave;
    BOOL mResized;
    
    /* Cached Objects */
    UIBezierPath * mPlotPath;
    UIBezierPath * mXAxisPath;
    UIBezierPath * mYAxisPath;
    
    AFPoint mLastOrigin;
    AFPoint mLastDimensions;
    CGFloat mYAxisNegativeStop;
    CGFloat mYAxisPositiveStop;
    BOOL mRecalculateAxis;
    UIBezierPath * mVerticalTickPath;
    UIBezierPath * mHorizontalTickPath;
    NSMutableArray * mTickLabelStrings;
    
    vector<CGPoint> * mTickLabelLocations;
}

// MARK: Initialization

-(id) initWithOriginAtX:(CGFloat)x
              originAtY:(CGFloat)y
                  width:(CGFloat)w
                 height:(CGFloat)h
{
    self = [super init];
    if (self)
    {
        self->mX = x;
        self->mY = y;
        self->mWidth = w;
        self->mHeight = h;
        mXAxisMargin = 0.05;
        mYAxisMargin = 0.1;
        mVerticalTickSpacing = 50.0;
        mHorizontalTickSpacing = 100.0;
        mTickLength = 5.0;
        mTitleYFraction = 0.1;
        
        mSineWave = new SineWave(1);
        mDataSource = mSineWave;
        
        [self updateTitleBounds];
        
        //mTitleFont = [NSFont fontWithName:@"Times New Roman" size: 10];
        mTitleFont = [UIFont fontWithName:@"Times New Roman" size:10];
        
        mTitle = nil;
        
        NSAssert(mTitleFont != nil, @"mTitleFont is nil!");
        
        NSMutableParagraphStyle * style;
        
        style = [[NSMutableParagraphStyle alloc] init];
        style.alignment = NSTextAlignmentCenter;
        
        {
            //NSString * keys[] = {NSFontFaceAttribute, NSParagraphStyleAttributeName};
            NSString * keys[] = {UIFontDescriptorFaceAttribute, NSParagraphStyleAttributeName};
            id objs[] = {mTitleFont, style};
            
            mAttributes = [NSDictionary dictionaryWithObjects:objs forKeys:keys count:2];
        }
        {
            //NSString * keys[] = {NSFontSizeAttribute};
            NSString * keys[] = {UIFontDescriptorSizeAttribute};
            
            id objs[] = {[NSNumber numberWithDouble:10.0]};
            
            mTickLabelFontAttributes = [NSDictionary dictionaryWithObjects:objs forKeys:keys count:1];
        }
        mTickLabelLocations = new vector<CGPoint>();
    }
    return self;
}

-(id) init
{
    return [self initWithOriginAtX:0
                         originAtY:0
                             width:0
                            height:0];
}

-(void) dealloc
{
    delete mSineWave;
    delete mTickLabelLocations;
}

// MARK: Public method implementation

-(void) startTimer
{
    NSTimeInterval msTimeInterval = mDataSource->get_updateInterval() / 1000.0;
    
    [mTimer invalidate];
    mTimer = [NSTimer scheduledTimerWithTimeInterval:msTimeInterval
                                              target:self
                                            selector:@selector(timeoutOccured:)
                                            userInfo:nil
                                             repeats:true];
}

-(void) stopTimer
{
    [mTimer invalidate];
    mTimer = nil;
}

-(void) useDemoSineWaveAsDataSource
{
    [self setDataSource:mSineWave];
}

-(void) setDemoSineWavePeriod:(int)period
{
    mSineWave->setPeriod(period);
}

-(void) timeoutOccured:(NSTimer*)timer
{
    //NSLog(@"%s\n", mDataSource->name());
    if (mDataSource->quit_requested())
    {
        [timer invalidate];
        mTimer = nil;
        mDataSource->acknowledge_quit();
        return;
    }
    if (mDataSource->valid() == false)
    {
        [_delegate redraw];
    }
}

-(void) setDataSource:(DataSourceRef)dataSource
{
    if (dataSource != NULL)
    {
        mDataSource = (DataSource *) dataSource;
        [self setTitle:[NSString stringWithFormat:@"%s",mDataSource->name()]];
        [self startTimer];
    }
}

-(void) setDimensionWidth:(CGFloat)width dimensionHeight:(CGFloat)height
{
    mWidth = width;
    mHeight = height;
    [self updateTitleBounds];
}

-(void) setOriginAtX:(CGFloat)x originAtY:(CGFloat)y
{
    mX = x;
    mY = y;
    [self updateTitleBounds];
}

-(void) setTitle:(NSString *)title
{
    mTitle = title;
    
    [self updateTitleBounds];
}

#define SQUARE(x) ((x) * (x))

bool shouldRecalculate(AFPoint origPoint, AFPoint newPoint, CGFloat xScale, CGFloat yScale)
{
    CGFloat xScreenPntsSquared = SQUARE(origPoint.x - newPoint.x) * xScale;
    CGFloat yScreenPntsSquared = SQUARE(origPoint.y - newPoint.y) * yScale;
    
    CGFloat distanceSquared = xScreenPntsSquared + yScreenPntsSquared;
    
    if (distanceSquared > 100.0)
    {
        printf("DistanceSquared: %.3f\n", distanceSquared);
        return true;
    }
    else
    {
        return false;
    }
}

-(void) draw:(CGContextRef) ctx
{
    AFPoint dimensions = mDataSource->get_lengths();
    if (dimensions.x < 1E-6 || dimensions.y < 1E-6) { return; }
    
    CGContextSaveGState(ctx);
    [self drawBoundingBox:ctx];
    [self drawTitle:ctx];
    [self drawPlot:ctx];
    CGContextRestoreGState(ctx);
}

// MARK: Private method implementation
-(void) drawTitle:(CGContextRef) ctx
{
    CGContextSaveGState(ctx);
    
    if (mTitle != nil)
    {
        [mTitle drawInRect:mTitleBounds withAttributes:mAttributes];
    }
    
    CGContextRestoreGState(ctx);
}

-(void) drawBoundingBox:(CGContextRef) ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 2.0);
    CGContextStrokeRect(ctx, CGRectMake(mX, mY, mWidth, mHeight));
    
    CGContextRestoreGState(ctx);
}

-(void) drawPlotAtOrigin:(CGContextRef) ctx
{
    CGContextSaveGState(ctx);
    
    if (mDataSource != NULL)
    {
        AFPoint lowerLeftHandCorner;
        AFPoint dimensions;
        size_t size;
        CGFloat plotCenterX;
        CGFloat plotCenterY;
        CGFloat translateByXToCenter;
        CGFloat translateByYToCenter;
        CGFloat xScale;
        CGFloat yScale;
        
        lowerLeftHandCorner = mDataSource->get_origin();
        dimensions = mDataSource->get_lengths();
        size = mDataSource->size();
        
        plotCenterX = lowerLeftHandCorner.x + dimensions.x / 2.0;
        plotCenterY = lowerLeftHandCorner.y + dimensions.y / 2.0;
        
        translateByXToCenter = -plotCenterX;
        translateByYToCenter = -plotCenterY;
        
        xScale = mWidth / dimensions.x;
        yScale = mHeight / dimensions.y;
        
        // Read transform instructions backwards, i.e. from bottom up
        CGContextSaveGState(ctx);
        CGContextScaleCTM(ctx, xScale, yScale);
        CGContextTranslateCTM(ctx, translateByXToCenter, translateByYToCenter);
        
        mRecalculateAxis = shouldRecalculate(mLastOrigin, lowerLeftHandCorner, xScale, yScale) ? YES : NO;
        mRecalculateAxis |= shouldRecalculate(mLastDimensions, dimensions, xScale, yScale) ? YES : NO;
        mRecalculateAxis = mResized ? YES : mRecalculateAxis;
        mResized = NO;
        
        mLastOrigin = lowerLeftHandCorner;
        mLastDimensions = dimensions;
        
        [self drawAxisTicks:ctx xScale:xScale yScale:yScale];
        
        [self drawAxis:ctx
           plotCenterX:plotCenterX
           plotCenterY:plotCenterY
                xScale:xScale
                yScale:yScale];
        
        if (mPlotPath == nil)
        {
            //mPlotPath = [[NSBezierPath alloc] init];
            mPlotPath = [[UIBezierPath alloc] init];
            mPlotPath.lineWidth = 0.5/yScale;
        }
        else
        {
            [mPlotPath removeAllPoints];
        }
        
        BOOL plotFirstPoint = YES;
        
        //CGColorRef grayColor = CGColorCreateGenericGray(0.15, 0.75);
        
        //CGColorRef black = NSColor.blackColor.CGColor;
        CGColorRef black = UIColor.blackColor.CGColor;
        
        CGContextSetStrokeColorWithColor(ctx, black);
        
        AFPoint pnt;
        CGPoint ns_pnt;
        
        int onZero = 0;
        
        
        for (int k = 0; k < size; k++)
        {
            if (onZero++%128 == 0)
            {
                pnt = mDataSource->get_data(k);
                ns_pnt.x = pnt.x;
                ns_pnt.y = pnt.y;
                
                if (plotFirstPoint == YES)
                {
                    plotFirstPoint = NO;
                    [mPlotPath moveToPoint:ns_pnt];
                }
                else
                {
                    //[mPlotPath lineToPoint:ns_pnt];
                    [mPlotPath addLineToPoint:ns_pnt];
                }
            }
        }
        
        [mPlotPath stroke];
        CGContextRestoreGState(ctx);
        
        
        [self drawAxisTickLabels:ctx
                          xScale:xScale
                          yScale:yScale
            translateByXToCenter:translateByXToCenter
            translateByYToCenter:translateByYToCenter];
        
    }
    
    CGContextRestoreGState(ctx);
}

-(void) drawAxis:(CGContextRef) ctx
     plotCenterX:(CGFloat) plotCenterX
     plotCenterY:(CGFloat) plotCenterY
          xScale:(CGFloat) xScale
          yScale:(CGFloat) yScale
{
    if (mDataSource != NULL)
    {
        CGContextSaveGState(ctx);
        
        AFPoint lowerLeftHandCorner;
        AFPoint dimensions;
        
        lowerLeftHandCorner = mDataSource->get_origin();
        dimensions = mDataSource->get_lengths();
        
        // Set the axis color
        //CGColorRef axisColor = NSColor.blueColor.CGColor;
        CGColorRef axisColor = UIColor.blueColor.CGColor;
        
        CGContextSetStrokeColorWithColor(ctx, axisColor);
        
        // Draw the x axis
        CGPoint xAxisFirstPoint;
        CGPoint xAxisLastPoint;
        
        if (mXAxisPath == nil)
        {
            //mXAxisPath = [[NSBezierPath alloc] init];
            mXAxisPath = [[UIBezierPath alloc] init];
        }
        else
        {
            [mXAxisPath removeAllPoints];
        }
        mXAxisPath.lineWidth = 1.0/yScale;
        
        //xAxisFirstPoint = NSMakePoint(lowerLeftHandCorner.x, plotCenterY);
        xAxisFirstPoint = CGPointMake(lowerLeftHandCorner.x, plotCenterY);
        
        //xAxisLastPoint = NSMakePoint(lowerLeftHandCorner.x + dimensions.x, plotCenterY);
        xAxisLastPoint = CGPointMake(lowerLeftHandCorner.x + dimensions.x, plotCenterY);
        
        [mXAxisPath moveToPoint:xAxisFirstPoint];
        
        //[mXAxisPath lineToPoint:xAxisLastPoint];
        [mXAxisPath addLineToPoint:xAxisLastPoint];
        
        [mXAxisPath stroke];
        
        // Draw the y axis
        CGPoint yAxisFirstPoint;
        CGPoint yAxisLastPoint;
        
        if (mYAxisPath == nil)
        {
            //mYAxisPath = [[NSBezierPath alloc] init];
            mYAxisPath = [[UIBezierPath alloc] init];
        }
        else
        {
            [mYAxisPath removeAllPoints];
        }
        mYAxisPath.lineWidth = 1.0/xScale;
        
        //yAxisFirstPoint = NSMakePoint(lowerLeftHandCorner.x, mYAxisNegativeStop);
        yAxisFirstPoint = CGPointMake(lowerLeftHandCorner.x, mYAxisNegativeStop);
        
        //yAxisLastPoint = NSMakePoint(lowerLeftHandCorner.x, mYAxisPositiveStop);
        yAxisLastPoint = CGPointMake(lowerLeftHandCorner.x, mYAxisPositiveStop);
        
        [mYAxisPath moveToPoint:yAxisFirstPoint];
        
        //[mYAxisPath lineToPoint:yAxisLastPoint];
        [mYAxisPath addLineToPoint:yAxisLastPoint];
        
        [mYAxisPath stroke];
        
        CGContextRestoreGState(ctx);
    }
}

-(void) drawAxisTickLabels:(CGContextRef) ctx
                    xScale:(CGFloat) xScale
                    yScale:(CGFloat) yScale
      translateByXToCenter:(CGFloat) xTranslation
      translateByYToCenter:(CGFloat) yTranslation
{
    if (mDataSource != NULL)
    {
        CGContextSaveGState(ctx);
        
        if (mTickLabelStrings == nil)
        {
            mTickLabelStrings = [[NSMutableArray alloc] init];
        }
        
        if (mRecalculateAxis)
        {
            [mTickLabelStrings removeAllObjects];
            mTickLabelLocations->clear();
            
            AFPoint lowerLeftHandCorner;
            AFPoint dimensions;
            CGFloat verticalTickSpacing;
            CGFloat horizontalTickSpacing;
            CGFloat intersectionWithHorizontalAxis;
            CGFloat k;
            CGFloat j;
            CGFloat upperYAxisLimit;
            CGFloat xPos;
            
            lowerLeftHandCorner = mDataSource->get_origin();
            dimensions = mDataSource->get_lengths();
            
            verticalTickSpacing = round(mVerticalTickSpacing / yScale * 10.0) / 10.0 * yScale;
            horizontalTickSpacing = round(mHorizontalTickSpacing / xScale * 10.0) / 10.0 * xScale;
            
            verticalTickSpacing = verticalTickSpacing >= 0.1 ? verticalTickSpacing : 0.1;
            horizontalTickSpacing = horizontalTickSpacing >= 0.1 ? horizontalTickSpacing : 0.1;
            
            intersectionWithHorizontalAxis = (lowerLeftHandCorner.y + dimensions.y/2.0 + yTranslation) * yScale;
            
            k = intersectionWithHorizontalAxis + verticalTickSpacing;
            j = intersectionWithHorizontalAxis - verticalTickSpacing;
            
            upperYAxisLimit = ((lowerLeftHandCorner.y + dimensions.y)+yTranslation) * yScale;
            
            xPos = (lowerLeftHandCorner.x + xTranslation)*xScale;
            
            while (k < upperYAxisLimit)
            {
                CGFloat tickPosition;
                NSString * text;
                CGSize size;
                CGPoint pnt;
                
                CGFloat negTickPosition;
                NSString * negText;
                CGSize negSize;
                
                tickPosition = round((k/yScale - yTranslation)*10.0)/10.0;
                text = [[NSNumber numberWithFloat:tickPosition] descriptionWithLocale:@"%.10f"];
                size = [text sizeWithAttributes:mTickLabelFontAttributes];
                
                //pnt = NSMakePoint(xPos - size.width - mTickLength, k - size.height/2.0);
                pnt = CGPointMake(xPos - size.width - mTickLength, k - size.height/2.0);
                
                //[text drawAtPoint:pnt withAttributes:mTickLabelFontAttributes];
                [mTickLabelStrings addObject:text];
                mTickLabelLocations->push_back(pnt);
                
                negTickPosition = round((j/yScale - yTranslation)*10.0)/10.0;
                
                negText = [[NSNumber numberWithFloat:negTickPosition] descriptionWithLocale:@"%.10f"];
                
                negSize = [negText sizeWithAttributes:mTickLabelFontAttributes];
                
                pnt.x = xPos - negSize.width - mTickLength;
                pnt.y = j - negSize.height;
                
                //[negText drawAtPoint:pnt withAttributes:mTickLabelFontAttributes];
                [mTickLabelStrings addObject:negText];
                mTickLabelLocations->push_back(pnt);
                
                k += verticalTickSpacing;
                j -= verticalTickSpacing;
            }
            
            CGFloat upperXAxisLimit = ((lowerLeftHandCorner.x + dimensions.x) + xTranslation)*xScale;
            
            // x coordinate intersection
            CGFloat intersectionWithVerticalAxis = (lowerLeftHandCorner.x + xTranslation) * xScale;
            
            CGFloat yPos = (lowerLeftHandCorner.y + dimensions.y/2.0 + yTranslation)*yScale;
            
            k = intersectionWithVerticalAxis;
            
            while (k < upperXAxisLimit)
            {
                CGFloat tickPosition;
                NSString * text;
                
                //NSPoint pnt = NSMakePoint(k, yPos);
                CGPoint pnt = CGPointMake(k, yPos);
                
                tickPosition = round((k/xScale - xTranslation)*10.0)/10.0;
                
                text = [[NSNumber numberWithFloat:tickPosition] descriptionWithLocale:@"%.1f"];
                
                //[text drawAtPoint:pnt withAttributes:mTickLabelFontAttributes];
                [mTickLabelStrings addObject:text];
                mTickLabelLocations->push_back(pnt);
                
                k += horizontalTickSpacing;
            }
        }
        
        NSInteger count = [mTickLabelStrings count];
        for (NSInteger k = 0; k < count; k ++)
        {
            NSString * text = [mTickLabelStrings objectAtIndex:k];
            CGPoint pnt = (*mTickLabelLocations)[k];
            
            [text drawAtPoint:pnt withAttributes:mTickLabelFontAttributes];
        }
        CGContextRestoreGState(ctx);
    }
}

-(void) drawAxisTicks:(CGContextRef) ctx
               xScale:(CGFloat) xScale
               yScale:(CGFloat) yScale
{
    if (mDataSource != NULL)
    {
        CGContextSaveGState(ctx);
        
        AFPoint lowerLeftHandCorner;
        AFPoint dimensions;
        CGFloat verticalTickSpacing;
        CGFloat horizontalTickSpacing;
        CGFloat intersectionWithHorizontalAxis;
        CGFloat tickLength;
        CGFloat k;
        CGFloat j;
        CGPoint leftPointAtPlotOrigin;
        CGPoint rightPointAtPlotOrigin;
        
        lowerLeftHandCorner = mDataSource->get_origin();
        dimensions = mDataSource->get_lengths();
        verticalTickSpacing = round(mVerticalTickSpacing/yScale*10.0)/10.0;
        horizontalTickSpacing = round(mHorizontalTickSpacing/xScale*10.0)/10.0;
        
        verticalTickSpacing = verticalTickSpacing >= 0.1 ? verticalTickSpacing : 0.1;
        horizontalTickSpacing = horizontalTickSpacing >= 0.1 ? horizontalTickSpacing : 0.1;
        
        intersectionWithHorizontalAxis = lowerLeftHandCorner.y+dimensions.y/2.0;
        tickLength = mTickLength/xScale;
        k = intersectionWithHorizontalAxis + verticalTickSpacing;
        j = intersectionWithHorizontalAxis - verticalTickSpacing;
        
        
        leftPointAtPlotOrigin.x = rightPointAtPlotOrigin.x = lowerLeftHandCorner.x + tickLength/2.0;
        
        if (mVerticalTickPath == nil)
        {
            mVerticalTickPath = [[UIBezierPath alloc] init];
            mVerticalTickPath.lineWidth = 1.0/yScale;
        }
        
        // Draw the positive and negative vertical ticks
        if (mRecalculateAxis)
        {
            [mVerticalTickPath removeAllPoints];
            
            leftPointAtPlotOrigin.x = lowerLeftHandCorner.x - tickLength/2.0;
            
            rightPointAtPlotOrigin.x = lowerLeftHandCorner.x + tickLength/2.0;
            
            CGFloat upperLimit = lowerLeftHandCorner.y + dimensions.y;
            
            while (k < upperLimit)
            {
                
                leftPointAtPlotOrigin.y = k;
                rightPointAtPlotOrigin.y = k;
                
                [mVerticalTickPath moveToPoint:leftPointAtPlotOrigin];
                
                //[mVerticalTickPath lineToPoint:rightPointAtPlotOrigin];
                [mVerticalTickPath addLineToPoint:rightPointAtPlotOrigin];
                
                leftPointAtPlotOrigin.y = j;
                rightPointAtPlotOrigin.y = j;
                
                [mVerticalTickPath moveToPoint:leftPointAtPlotOrigin];
                
                //[mVerticalTickPath lineToPoint:rightPointAtPlotOrigin];
                [mVerticalTickPath addLineToPoint:rightPointAtPlotOrigin];
                
                k += verticalTickSpacing;
                j -= verticalTickSpacing;
            }
            
            mYAxisPositiveStop = k - verticalTickSpacing;
            mYAxisNegativeStop = j + verticalTickSpacing;
        }
        
        
        [mVerticalTickPath stroke];
        
        // Draw the horizontal axis
        CGPoint upperPoint;
        CGPoint lowerPoint;
        
        if (mHorizontalTickPath == nil)
        {
            mHorizontalTickPath = [[UIBezierPath alloc] init];
            mHorizontalTickPath.lineWidth = 2.0/xScale;
        }
        
        tickLength = 5.0/yScale;
        
        if (mRecalculateAxis)
        {
            [mHorizontalTickPath removeAllPoints];
            
            upperPoint.y = intersectionWithHorizontalAxis + tickLength/2.0;
            lowerPoint.y = intersectionWithHorizontalAxis - tickLength/2.0;
            
            k = lowerLeftHandCorner.x;
            
            while (k < lowerLeftHandCorner.x + dimensions.x)
            {
                upperPoint.x = k;
                lowerPoint.x = k;
                
                [mHorizontalTickPath moveToPoint:upperPoint];
                [mHorizontalTickPath addLineToPoint:lowerPoint];
                
                k += horizontalTickSpacing;
            }
            
        }
        
        [mHorizontalTickPath stroke];
        
        CGContextRestoreGState(ctx);
    }
}

-(void) drawPlot:(CGContextRef) ctx
{
    CGContextSaveGState(ctx);
    
    CGFloat xMargin;
    
    xMargin = mXAxisMargin * mWidth;
    
    CGContextTranslateCTM(ctx, xMargin/2.0+mX+mWidth/2.0, mY+mHeight/2.0);
    CGContextScaleCTM(ctx, 1.0-mXAxisMargin, 0.9);
    [self drawPlotAtOrigin:ctx];
    CGContextRestoreGState(ctx);
}

-(void) updateTitleBounds
{
    CGFloat titleX;
    CGFloat titleY;
    CGFloat titleWidth;
    CGFloat titleHeight;
    
    titleX = mX;
    titleY = mY + mHeight * (1.0 - mTitleYFraction);
    titleWidth = mWidth;
    titleHeight = mHeight * mTitleYFraction;
    
    mTitleBounds = CGRectMake(titleX, titleY, titleWidth, titleHeight);
    mResized = YES;
}

@end
