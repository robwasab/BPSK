//
//  Graph.swift
//  CoreGraphicsMac
//
//  Created by Robby Tong on 9/22/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

import Foundation
import AppKit

protocol Plottable
{
    func getPlotLowerLeftHandCorner()->CGPoint
    func getPlotDimensions()->CGSize
    func getDataSize()->Int
    func getPoint(index:Int)->CGPoint
}

class GraphGroup
{
    private let mNumGraphs:Int
    private var mWidth:CGFloat
    private var mHeight:CGFloat
    private var mExpandRemainingGraphs:Bool
    
    // column of graphs
    private var mGraphs:[Graph] = []
    
    init(numGraphs:Int, width:CGFloat, height:CGFloat, expandRemainingGraphs:Bool)
    {
        mNumGraphs = numGraphs
        //mGraphs = Array<Graph>(repeating: Graph(), count: numGraphs)
        for _ in 0..<numGraphs
        {
            mGraphs.append(Graph())
        }
        mExpandRemainingGraphs = expandRemainingGraphs
        mWidth = width
        mHeight = height
        layoutGraphs()
    }
    
    func allowGraphsToExapand(_ expandRemainingGraphs:Bool)
    {
        mExpandRemainingGraphs = expandRemainingGraphs
        layoutGraphs()
    }
    
    func setDimensions(rect:NSRect)
    {
        mWidth = rect.width
        mHeight = rect.height
        layoutGraphs()
    }
    
    func setDimensions(width:CGFloat, height:CGFloat)
    {
        mWidth = width
        mHeight = height
        layoutGraphs()
    }
    
    func layoutGraphs()
    {
        let numColumns = mNumGraphs / 4 + (mNumGraphs % 4 > 0 ? 1 : 0)
        
        let columnWidth = mWidth / CGFloat(Float(numColumns))
        
        for k in 0..<mNumGraphs
        {
            let graphWidth = columnWidth
            var graphHeight = mHeight / 4.0
            
            let graphX = CGFloat(Float(k / 4)) * graphWidth
            var graphY = CGFloat(Float(k % 4)) * graphHeight
            
            if mExpandRemainingGraphs == true
            {
                let numberOfGraphsInLastColumn = mNumGraphs % 4
                if k >= mNumGraphs - numberOfGraphsInLastColumn
                {
                    graphHeight = mHeight / CGFloat(Float(numberOfGraphsInLastColumn))
                    graphY = graphHeight * CGFloat(Float(k % 4))
                }
            }
            print("graphWidth: \(graphWidth)")
            print("graphHeight: \(graphHeight)")
            print("graphX: \(graphX)")
            print("graphY: \(graphY)")
            mGraphs[k].setDimensions(width: graphWidth, height: graphHeight)
            mGraphs[k].setOrigin(x: graphX, y: graphY)
            //mGraphs.append(Graph(x: graphX, y: graphY, width: graphWidth, height: graphHeight))
        }
    }
    
    func getGraphs() -> [Graph]
    {
        return mGraphs
    }
    
    func draw(ctx:CGContext)
    {
        for graph in mGraphs
        {
            graph.draw(ctx:ctx)
        }
    }
}

class Graph
{
    var mX:CGFloat
    var mY:CGFloat
    var mWidth:CGFloat
    var mHeight:CGFloat
    var mDelegate:Plottable?

    
    var mTitleFont:NSFont?
    var mTitle:NSString?
    var mTitleBounds:CGRect
    
    let mXAxisMargin:CGFloat = 0.05
    let mYAxisMargin:CGFloat = 0.1
    
    let mVerticalTickSpacing:CGFloat = 50.0
    let mHorizontalTickSpacing:CGFloat = 100.0
    let mTickLength:CGFloat = 5.0
    
    required init(x:CGFloat, y:CGFloat, width:CGFloat, height:CGFloat)
    {
        self.mX = x
        self.mY = y
        self.mWidth = width
        self.mHeight = height
        
        let titleX = mX
        let titleY = mY + mHeight
        let titleWidth = mWidth
        let titleHeight = mHeight * 0.25
        
        self.mTitleBounds = CGRect(x: titleX, y: titleY, width: titleWidth, height: titleHeight)
        
        if let font = NSFont(name: "Times New Roman", size: 10)
        {
            mTitleFont = font
        }
        else
        {
            assertionFailure("NSFont returned nil, probably could not find font")
        }
    }
    
    convenience init()
    {
        self.init(x: 0, y: 0, width: 0, height: 0)
    }
    
    func setDimensions(width:CGFloat, height:CGFloat)
    {
        mWidth = width
        mHeight = height
        updateTitleBounds()
    }
    
    func setOrigin(x:CGFloat, y:CGFloat)
    {
        mX = x
        mY = y
        updateTitleBounds()
    }
    
    func setTitle(title:NSString)
    {
        mTitle = title
        updateTitleBounds()
    }
    
    func draw(ctx:CGContext)
    {
        ctx.saveGState()
        
        drawBoundingBox(ctx)
        
        drawTitle(ctx)
        
        drawPlot(ctx)
        
        ctx.restoreGState()
    }
    
    private func drawTitle(_ ctx:CGContext)
    {
        ctx.saveGState()
        
        if let title = mTitle
        {
            if let font = mTitleFont
            {
                let style = NSMutableParagraphStyle()
                style.alignment = .center
                
                let attributes:[String:Any] = [NSFontFaceAttribute: font, NSParagraphStyleAttributeName: style]
                title.draw(in: mTitleBounds, withAttributes: attributes)
            }
        }
        ctx.restoreGState()
    }
    
    private func drawBoundingBox(_ ctx:CGContext)
    {
        ctx.saveGState()
        
        let bounds = CGRect(x: mX, y: mY, width: mWidth, height: mHeight)
        //print("Drawing Graph, origin: [\(mX), \(mY)], dimensions: [\(mWidth), \(mHeight)]")
        
        ctx.setLineWidth(2.0)
        ctx.stroke(bounds)
        
        ctx.restoreGState()
    }
    
    private func drawPlotAtOrigin(_ ctx:CGContext)
    {
        ctx.saveGState()
        
        if let delegate = mDelegate
        {
            let lowerLeftHandCorner = delegate.getPlotLowerLeftHandCorner()
            let dimensions = delegate.getPlotDimensions()
            let size = delegate.getDataSize()
            
            let plotCenterX = lowerLeftHandCorner.x + dimensions.width / 2.0
            let plotCenterY = lowerLeftHandCorner.y + dimensions.height / 2.0
            
            let translateByXToCenter = -plotCenterX
            let translateByYToCenter = -plotCenterY
            
            let xScale = mWidth / dimensions.width
            let yScale = mHeight / dimensions.height
            
            // Read transform instructions backwards, i.e. from bottom up
            ctx.saveGState()
            ctx.scaleBy(x: xScale, y: yScale)
            ctx.translateBy(x: translateByXToCenter, y: translateByYToCenter)
            
            if let (yAxisNegativePartStop, yAxisPositivePartStop) = drawAxisTicks(ctx,
                                                        xScale: xScale,
                                                        yScale: yScale)
            {
                drawAxis(ctx,
                         plotCenterX: plotCenterX,
                         plotCenterY: plotCenterY,
                         xScale: xScale,
                         yScale: yScale,
                         yAxisPositivePartStopsAt: yAxisPositivePartStop,
                         yAxisNegativePartStopsAt: yAxisNegativePartStop)
            }
            else
            {
            }
            
            // Draw the data
            let path = NSBezierPath()
            var plotFirstPoint = true
            
            let grayColor = CGColor(gray: 0.15, alpha: 0.75)
            
            ctx.setStrokeColor(grayColor)
            path.lineWidth = 2.0/yScale
            
            for k in 0..<size
            {
                let pnt = delegate.getPoint(index: k)
                /*
                 pnt.x += translateByXToCenter
                 pnt.y += translateByYToCenter
                 pnt.x *= xScale
                 pnt.y *= yScale
                 pnt.x += mX + mWidth/2.0
                 pnt.y += mY + mHeight/2.0
                 */
                if plotFirstPoint
                {
                    plotFirstPoint = false
                    path.move(to: pnt)
                }
                else
                {
                    path.line(to: pnt)
                }
            }
            path.stroke()
            ctx.restoreGState()
            drawAxisTickLabels(ctx,
                               xScale: xScale,
                               yScale: yScale,
                               translateByXToCenter: translateByXToCenter,
                               translateByYToCenter: translateByYToCenter)
        }
        ctx.restoreGState()
    }
    
    private func drawAxis(_ ctx:CGContext,
                          plotCenterX:CGFloat,
                          plotCenterY:CGFloat,
                          xScale:CGFloat,
                          yScale:CGFloat,
                          yAxisPositivePartStopsAt yAxisPositiveStop:CGFloat,
                          yAxisNegativePartStopsAt yAxisNegativeStop:CGFloat)
    {
        if let delegate = mDelegate
        {
            ctx.saveGState()

            let lowerLeftHandCorner = delegate.getPlotLowerLeftHandCorner()
            let dimensions = delegate.getPlotDimensions()

            // Set the axis color
            let axisColor = NSColor.blue.cgColor
            ctx.setStrokeColor(axisColor)
        
            // Draw the x axis
            let xAxisPath = NSBezierPath()
            let xAxisFirstPoint = NSPoint(x: lowerLeftHandCorner.x, y: plotCenterY)
            let xAxisLastPoint = NSPoint(x: lowerLeftHandCorner.x + dimensions.width, y: plotCenterY)
        
            xAxisPath.lineWidth = 1.0/yScale
            xAxisPath.move(to: xAxisFirstPoint)
            xAxisPath.line(to: xAxisLastPoint)
            xAxisPath.stroke()
        
            // Draw the y axis
            let yAxisPath = NSBezierPath()
            let yAxisFirstPoint = NSPoint(x: lowerLeftHandCorner.x,
                                          y:yAxisNegativeStop)
            let yAxisLastPoint = NSPoint(x: lowerLeftHandCorner.x,
                                         y:yAxisPositiveStop)
        
            yAxisPath.lineWidth = 1.0/xScale
            yAxisPath.move(to: yAxisFirstPoint)
            yAxisPath.line(to: yAxisLastPoint)
            yAxisPath.stroke()
        
            ctx.restoreGState()
        }
    }
    
    private func drawAxisTickLabels(_ ctx:CGContext,
                                    xScale:CGFloat,
                                    yScale:CGFloat,
                                    translateByXToCenter:CGFloat,
                                    translateByYToCenter:CGFloat)
    {
        if let delegate = mDelegate
        {
            ctx.saveGState()
            let lowerLeftHandCorner = delegate.getPlotLowerLeftHandCorner()
            let dimensions = delegate.getPlotDimensions()

            let verticalTickSpacing
                = (mVerticalTickSpacing / yScale * 10.0).rounded() / 10.0 * yScale
            
            let horizontalTickSpacing
                = (mHorizontalTickSpacing / xScale * 10.0).rounded() / 10.0 * xScale
            
            let intersectionWithHorizontalAxis
                = (lowerLeftHandCorner.y + dimensions.height/2.0 + translateByYToCenter) * yScale
        
            var k = intersectionWithHorizontalAxis + verticalTickSpacing
            var j = intersectionWithHorizontalAxis - verticalTickSpacing
            
            let upperYAxisLimit
                = ((lowerLeftHandCorner.y + dimensions.height) + translateByYToCenter)*yScale
            
            let xPos = (lowerLeftHandCorner.x + translateByXToCenter)*xScale
            
            let attributes:[String:Any] = [
                NSFontSizeAttribute: NSNumber(value:10.0)
            ]
            
            while k < upperYAxisLimit
            {
                let tickPosition = ((k/yScale - translateByYToCenter) * 10.0).rounded()/10.0
                
                let text = tickPosition.description as NSString
                let size = text.size(withAttributes: attributes)
                
                var pnt = NSPoint(x: xPos - size.width - mTickLength, y:k - size.height/2.0)
                
                text.draw(at: pnt, withAttributes: attributes)

                let negTickPosition = j / yScale - translateByYToCenter
                
                let negText = negTickPosition.description as NSString
                let negSize = negText.size(withAttributes: attributes)
                
                pnt.x = xPos - negSize.width - mTickLength
                pnt.y = j - negSize.height/2.0
                
                negText.draw(at: pnt, withAttributes: attributes)
                
                k += verticalTickSpacing
                j -= verticalTickSpacing
            }
            
            let upperXAxisLimit
                = ((lowerLeftHandCorner.x + dimensions.width) + translateByXToCenter)*xScale
            
            let intersectionWithVerticalAxis
                = (lowerLeftHandCorner.x + translateByXToCenter) * xScale
            
            let yPos = (lowerLeftHandCorner.y + dimensions.height/2.0 + translateByYToCenter)*yScale

            k = intersectionWithVerticalAxis
            
            while k < upperXAxisLimit
            {
                let tickPosition = ((k/xScale - translateByXToCenter)*10.0).rounded()/10.0
                
                let text = tickPosition.description as NSString
                //let size = text.size(withAttributes: attributes)
                
                let pnt = NSPoint(x: k, y: yPos)
                
                text.draw(at: pnt, withAttributes: attributes)
                
                k += horizontalTickSpacing
            }
            
            ctx.restoreGState()
        }
    }
    
    private func drawAxisTicks(_ ctx:CGContext, xScale:CGFloat, yScale:CGFloat) -> (CGFloat,CGFloat)?
    {
        if let delegate = mDelegate
        {
            ctx.saveGState()
            
            let lowerLeftHandCorner = delegate.getPlotLowerLeftHandCorner()
            let dimensions = delegate.getPlotDimensions()

            // want a tick every 50.0 points of screen
            // divide by yScale to convert into plot cordinates
            let verticalTickSpacing = (mVerticalTickSpacing / yScale * 10.0).rounded() / 10.0
            let horizontalTickSpacing = (mHorizontalTickSpacing / xScale * 10.0).rounded() / 10.0
            
            let intersectionWithHorizontalAxis = lowerLeftHandCorner.y + dimensions.height/2.0
        
            var tickLength:CGFloat = mTickLength/xScale
        
            let verticalTick = NSBezierPath()
        
            verticalTick.lineWidth = 2.0/yScale
        
            var k = intersectionWithHorizontalAxis + verticalTickSpacing
            var j = intersectionWithHorizontalAxis - verticalTickSpacing
            
            var leftPointAtPlotOrigin = NSPoint()
            var rightPointAtPlotOrigin = NSPoint()
            
            leftPointAtPlotOrigin.x = lowerLeftHandCorner.x - tickLength/2.0
            rightPointAtPlotOrigin.x = lowerLeftHandCorner.x + tickLength/2.0
            
            // Draw the positive and negative vertical ticks
            while k < lowerLeftHandCorner.y + dimensions.height
            {
                leftPointAtPlotOrigin.y = k
                rightPointAtPlotOrigin.y = k
                
                verticalTick.move(to: leftPointAtPlotOrigin)
                verticalTick.line(to: rightPointAtPlotOrigin)
            
                leftPointAtPlotOrigin.y = j
                rightPointAtPlotOrigin.y = j
                
                verticalTick.move(to: leftPointAtPlotOrigin)
                verticalTick.line(to: rightPointAtPlotOrigin)
                
                k += verticalTickSpacing
                j -= verticalTickSpacing
            }
            
            let yAxisPositivePartStop:CGFloat = k - verticalTickSpacing
            let yAxisNegativePartStop:CGFloat = j + verticalTickSpacing
            
            verticalTick.stroke()
            
            // Draw the horizontal ticks
            let horizontalTick = NSBezierPath()
            horizontalTick.lineWidth = 2.0/xScale
            tickLength = 5.0/yScale
            
            var upperPoint = NSPoint()
            var lowerPoint = NSPoint()

            upperPoint.y = intersectionWithHorizontalAxis + tickLength/2.0
            lowerPoint.y = intersectionWithHorizontalAxis - tickLength/2.0
            
            k = lowerLeftHandCorner.x
            
            while k < lowerLeftHandCorner.x + dimensions.width
            {
                upperPoint.x = k
                lowerPoint.x = k
                
                horizontalTick.move(to: upperPoint)
                horizontalTick.line(to: lowerPoint)
                
                k += horizontalTickSpacing
            }
            horizontalTick.stroke()
            ctx.restoreGState()
            return (yAxisNegativePartStop, yAxisPositivePartStop)
        }
        return nil
    }
    
    private func drawPlot(_ ctx:CGContext)
    {
        ctx.saveGState()
        
        let xMargin = mXAxisMargin*mWidth
        //let yMargin = mYAxisMargin*mHeight
        //ctx.translateBy(x: xMargin/2.0+mX+mWidth/2.0, y: yMargin/2.0+mY+mHeight/2.0)
        //ctx.scaleBy(x: 1.0-mXAxisMargin, y: 1.0-mYAxisMargin)
        ctx.translateBy(x: xMargin/2.0+mX+mWidth/2.0, y: mY+mHeight/2.0)
        ctx.scaleBy(x: 1.0-mXAxisMargin, y: 1.0)
        
        drawPlotAtOrigin(ctx)
        
        ctx.restoreGState()
    }
    
    private func updateTitleBounds()
    {
        let titleYFraction:CGFloat = 0.1
        let titleX = mX
        let titleY = mY + mHeight * (1.0 - titleYFraction)
        let titleWidth = mWidth
        let titleHeight = mHeight * titleYFraction
        
        self.mTitleBounds = CGRect(x: titleX, y: titleY, width: titleWidth, height: titleHeight)
    }
}

class SineWave: Plottable
{
    let mPeriods:Double
    init(periods:Int)
    {
        mPeriods = Double(periods)
    }
    func getPlotLowerLeftHandCorner()->CGPoint
    {
        return CGPoint(x: 0, y: -1.5)
    }
    func getPlotDimensions()->CGSize
    {
        return CGSize(width: mPeriods * 2.0, height: 3.0)
    }
    func getDataSize()->Int
    {
        return 256
    }
    func getPoint(index:Int)->CGPoint
    {
        let x = Double(index) / 256.0 * mPeriods * 2.0 * Double.pi
        let y = sin(x)
        return CGPoint(x: x/Double.pi, y: y)
    }
}


