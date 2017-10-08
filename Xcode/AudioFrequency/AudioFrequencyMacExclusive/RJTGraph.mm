//
//  Graph.m
//  CoreGraphicsMac
//
//  Created by Robby Tong on 10/2/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/AppKit.h>
#import "RJTGraph.h"
#include "DataSource.h"
#include "SineWave.hpp"

@interface RJTGraph()

// MARK: Private Methods

-(void) drawTitle:(CGContextRef) ctx;
-(void) drawBoundingBox:(CGContextRef) ctx;
-(void) drawPlotAtOrigin:(CGContextRef) ctx;

-(void) drawAxis:(CGContextRef) ctx
     plotCenterX:(CGFloat) plotCenterX
     plotCenterY:(CGFloat) plotCenterY
          xScale:(CGFloat) xScale
          yScale:(CGFloat) yScale
yAxisPositivePartStopsAt:(CGFloat) yAxisPositiveStop
yAxisNegativePartStopsAt:(CGFloat) yAxisNegativeStop;

-(void) drawAxisTickLabels:(CGContextRef) ctx
                    xScale:(CGFloat) xScale
                    yScale:(CGFloat) yScale
      translateByXToCenter:(CGFloat) xTranslation
      translateByYToCenter:(CGFloat) yTranslation;

-(void) drawAxisTicks:(CGContextRef) ctx
               xScale:(CGFloat) xScale
               yScale:(CGFloat) yScale
 outYAxisNegativeStop:(CGFloat *) negativeStopRef
 outYAxisPositiveStop:(CGFloat *) positiveStopRef;

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
    NSFont * mTitleFont;
    NSString * mTitle;
    NSDictionary * mAttributes;
    NSDictionary * mTickLabelFontAttributes;
    NSTimer * mTimer;
    DataSource * mDataSource;
    SineWave * mSineWave;
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
        
        //[self startTimer];
        [self updateTitleBounds];
        
        mTitleFont = [NSFont fontWithName:@"Times New Roman" size: 10];
        
        mTitle = nil;
        
        NSAssert(mTitleFont != nil, @"mTitleFont is nil!");
        
        NSMutableParagraphStyle * style;
        
        style = [[NSMutableParagraphStyle alloc] init];
        style.alignment = NSTextAlignmentCenter;
        
        {
            NSString * keys[] = {NSFontFaceAttribute, NSParagraphStyleAttributeName};
            id objs[] = {mTitleFont, style};
            
            mAttributes = [NSDictionary dictionaryWithObjects:objs forKeys:keys count:2];
        }
        {
            NSString * keys[] = {NSFontSizeAttribute};
            id objs[] = {[NSNumber numberWithDouble:10.0]};
            
            mTickLabelFontAttributes = [NSDictionary dictionaryWithObjects:objs forKeys:keys count:1];
        }
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

-(void) draw:(CGContextRef) ctx
{
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
        
        CGFloat yAxisNegativeStop = 0;
        CGFloat yAxisPositiveStop = 0;
        
        [self drawAxisTicks:ctx xScale:xScale
                     yScale:yScale
       outYAxisNegativeStop:&yAxisNegativeStop
       outYAxisPositiveStop:&yAxisPositiveStop];
        
        [self drawAxis:ctx
           plotCenterX:plotCenterX
           plotCenterY:plotCenterY
                xScale:xScale
                yScale:yScale
yAxisPositivePartStopsAt:yAxisPositiveStop
yAxisNegativePartStopsAt:yAxisNegativeStop];
        
        NSBezierPath * path = [[NSBezierPath alloc] init];
        BOOL plotFirstPoint = YES;
        
        //CGColorRef grayColor = CGColorCreateGenericGray(0.15, 0.75);
        
        CGColorRef black = NSColor.blackColor.CGColor;
        
        CGContextSetStrokeColorWithColor(ctx, black);
        path.lineWidth = 2.0/yScale;
        
        AFPoint pnt;
        NSPoint ns_pnt;
        
        int onZero = 0;
        
        for (int k = 0; k < size; k++)
        {
            if (onZero++%32 == 0)
            {
                pnt = mDataSource->get_data(k);
                ns_pnt.x = pnt.x;
                ns_pnt.y = pnt.y;
            
                if (plotFirstPoint == YES)
                {
                    plotFirstPoint = NO;
                    [path moveToPoint:ns_pnt];
                }
                else
                {
                    [path lineToPoint:ns_pnt];
                }
            }
        }
        [path stroke];
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
yAxisPositivePartStopsAt:(CGFloat) yAxisPositiveStop
yAxisNegativePartStopsAt:(CGFloat) yAxisNegativeStop
{
    if (mDataSource != NULL)
    {
        CGContextSaveGState(ctx);
        
        AFPoint lowerLeftHandCorner;
        AFPoint dimensions;
        
        lowerLeftHandCorner = mDataSource->get_origin();
        dimensions = mDataSource->get_lengths();
        
        // Set the axis color
        CGColorRef axisColor = NSColor.blueColor.CGColor;
        CGContextSetStrokeColorWithColor(ctx, axisColor);
        
        // Draw the x axis
        NSBezierPath * xAxisPath;
        NSPoint xAxisFirstPoint;
        NSPoint xAxisLastPoint;
        
        xAxisPath = [[NSBezierPath alloc] init];
        xAxisFirstPoint = NSMakePoint(lowerLeftHandCorner.x, plotCenterY);
        xAxisLastPoint = NSMakePoint(lowerLeftHandCorner.x + dimensions.x, plotCenterY);
        
        xAxisPath.lineWidth = 1.0/yScale;
        [xAxisPath moveToPoint:xAxisFirstPoint];
        [xAxisPath lineToPoint:xAxisLastPoint];
        [xAxisPath stroke];
        
        // Draw the y axis
        NSBezierPath * yAxisPath;
        NSPoint yAxisFirstPoint;
        NSPoint yAxisLastPoint;
        
        yAxisPath = [[NSBezierPath alloc] init];
        yAxisFirstPoint = NSMakePoint(lowerLeftHandCorner.x, yAxisNegativeStop);
        yAxisLastPoint = NSMakePoint(lowerLeftHandCorner.x, yAxisPositiveStop);
        
        yAxisPath.lineWidth = 1.0/xScale;
        [yAxisPath moveToPoint:yAxisFirstPoint];
        [yAxisPath lineToPoint:yAxisLastPoint];
        [yAxisPath stroke];
        
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
            NSPoint pnt;
            
            CGFloat negTickPosition;
            NSString * negText;
            CGSize negSize;
            
            tickPosition = round((k/yScale - yTranslation)*10.0)/10.0;
            text = [[NSNumber numberWithFloat:tickPosition] descriptionWithLocale:@"%.1f"];
            size = [text sizeWithAttributes:mTickLabelFontAttributes];
            
            pnt = NSMakePoint(
            xPos - size.width - mTickLength, k - size.height/2.0);
            
            [text drawAtPoint:pnt withAttributes:mTickLabelFontAttributes];
            
            negTickPosition = j/yScale - yTranslation;
            
            negText = [[NSNumber numberWithFloat:negTickPosition] descriptionWithLocale:@"%.1f"];
            
            negSize = [negText sizeWithAttributes:mTickLabelFontAttributes];
            
            pnt.x = xPos - negSize.width - mTickLength;
            pnt.y = j - negSize.height;
            
            [negText drawAtPoint:pnt withAttributes:mTickLabelFontAttributes];
            
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
            NSPoint pnt = NSMakePoint(k, yPos);
            
            tickPosition = round((k/xScale - xTranslation)*10.0)/10.0;
            
            text = [[NSNumber numberWithFloat:tickPosition] descriptionWithLocale:@"%.1f"];
            
            [text drawAtPoint:pnt withAttributes:mTickLabelFontAttributes];
            
            k += horizontalTickSpacing;
        }
        CGContextRestoreGState(ctx);
    }
}

-(void) drawAxisTicks:(CGContextRef) ctx
               xScale:(CGFloat) xScale
               yScale:(CGFloat) yScale
 outYAxisNegativeStop:(CGFloat *) negativeStopRef
 outYAxisPositiveStop:(CGFloat *) positiveStopRef
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
        NSPoint leftPointAtPlotOrigin;
        NSPoint rightPointAtPlotOrigin;
        NSBezierPath * verticalTick;
        
        lowerLeftHandCorner = mDataSource->get_origin();
        dimensions = mDataSource->get_lengths();
        verticalTickSpacing = round(mVerticalTickSpacing/yScale*10.0)/10.0;
        horizontalTickSpacing = round(mHorizontalTickSpacing/xScale*10.0)/10.0;
        intersectionWithHorizontalAxis = lowerLeftHandCorner.y+dimensions.y/2.0;
        tickLength = mTickLength/xScale;
        k = intersectionWithHorizontalAxis + verticalTickSpacing;
        j = intersectionWithHorizontalAxis - verticalTickSpacing;
        
        
        leftPointAtPlotOrigin.x =         rightPointAtPlotOrigin.x = lowerLeftHandCorner.x + tickLength/2.0;
        
        verticalTick = [[NSBezierPath alloc] init];
        
        verticalTick.lineWidth = 2.0/yScale;
        
        // Draw the positive and negative vertical ticks
        while (k < lowerLeftHandCorner.y + dimensions.y)
        {
            leftPointAtPlotOrigin =
            NSMakePoint(lowerLeftHandCorner.x - tickLength/2.0, k);
            
            rightPointAtPlotOrigin =
            NSMakePoint(lowerLeftHandCorner.x + tickLength/2.0, k);
            
            [verticalTick moveToPoint:leftPointAtPlotOrigin];
            [verticalTick lineToPoint:rightPointAtPlotOrigin];
            
            leftPointAtPlotOrigin.y = j;
            rightPointAtPlotOrigin.y = j;
            
            [verticalTick moveToPoint:leftPointAtPlotOrigin];
            [verticalTick lineToPoint:rightPointAtPlotOrigin];
            
            k += verticalTickSpacing;
            j -= verticalTickSpacing;
        }
        
        if (negativeStopRef != NULL &&
            positiveStopRef != NULL)
        {
            *positiveStopRef = k - verticalTickSpacing;
            *negativeStopRef = j + verticalTickSpacing;
        }
        
        [verticalTick stroke];
        
        // Draw the horizontal axis
        NSBezierPath * horizontalTick;
        NSPoint upperPoint;
        NSPoint lowerPoint;
        
        horizontalTick = [[NSBezierPath alloc] init];
        horizontalTick.lineWidth = 2.0/xScale;
        tickLength = 5.0/yScale;
        
        upperPoint.y = intersectionWithHorizontalAxis + tickLength/2.0;
        lowerPoint.y = intersectionWithHorizontalAxis - tickLength/2.0;
        
        k = lowerLeftHandCorner.x;
        
        while (k < lowerLeftHandCorner.x + dimensions.x)
        {
            upperPoint.x = k;
            lowerPoint.x = k;
            
            [horizontalTick moveToPoint:upperPoint];
            [horizontalTick lineToPoint:lowerPoint];
            
            k += horizontalTickSpacing;
        }
        
        [horizontalTick stroke];
        
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
}

@end
