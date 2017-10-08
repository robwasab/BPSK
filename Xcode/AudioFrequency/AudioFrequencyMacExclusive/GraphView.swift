//
//  GraphView.swift
//  CoreGraphicsMac
//
//  Created by Robby Tong on 9/17/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

import Cocoa
import CoreGraphics

@IBDesignable class GraphView: NSView, RJTViewUpdateDelegate {

    var mGraphGroup:RJTGraphGroup?
    var mRadio:AudioFrequencyWrapper?
    
    @IBInspectable var expandRemainingGraphs:Bool = false
    {
        didSet
        {
            mGraphGroup?.graphsShouldExpand(expandRemainingGraphs)
            //mGraphGroup?.allowGraphsToExapand(expandRemainingGraphs)
            needsDisplay = true
        }
    }
    
    
    @IBInspectable var numGraphs:Int = 3
    {
        didSet
        {
            //updateGraphGroup()
            needsDisplay = true
        }
    }
    
    override var bounds:NSRect
    {
        didSet
        {
            mGraphGroup?.setDimensionsWithRect(bounds.size)
            //mGraphGroup?.setDimensions(rect: bounds)
            needsDisplay = true
        }
    }
    
    
    required init?(coder: NSCoder)
    {
        super.init(coder: coder)
        updateGraphGroup()
        mRadio = AudioFrequencyWrapper(blockOnStart: false)
        mRadio?.start()
    }
    
    override init(frame:NSRect)
    {
        super.init(frame:frame)
    }
    
    func redraw()
    {
        self.needsDisplay = true
    }
    
    func updateGraphGroup()
    {
        let width = self.bounds.width
        let height = self.bounds.height
        
        mGraphGroup = RJTGraphGroup(numGraphs: Int32(numGraphs),
                                    width: width,
                                    height: height,
                                    expandRemainingGraphs: expandRemainingGraphs)
        mGraphGroup?.delegate = self
        /*
        mGraphGroup = GraphGroup(
            numGraphs: numGraphs,
            width: width,
            height: height,
            expandRemainingGraphs: expandRemainingGraphs)
         */
        self.needsDisplay = true
    }
    
    func setOrigin(x:CGFloat, y:CGFloat)
    {
        self.needsDisplay = true
    }
    
    override func viewDidEndLiveResize()
    {
        super.viewDidEndLiveResize()
        //Swift.print("\(self.bounds)")
        mGraphGroup?.setDimensionsWithRect(self.bounds.size)
        //mGraphGroup?.setDimensions(rect: self.bounds)
        self.needsDisplay = true
    }
    
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)
        
        guard let ctx = NSGraphicsContext.current()?.cgContext else {return}
        ctx.setFillColor(NSColor.white.cgColor)
        ctx.addRect(self.bounds as CGRect)
        ctx.fill(self.bounds as CGRect)

        mGraphGroup?.draw(ctx)
        //mGraphGroup?.draw(ctx: ctx)
    }
}
