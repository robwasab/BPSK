//
//  GraphView.swift
//  CoreGraphicsMac
//
//  Created by Robby Tong on 9/17/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

import UIKit
import Foundation
import CoreGraphics

@IBDesignable class GraphView: UIView, RJTViewUpdateDelegate, RJTRadioReceiveProtocol {
    
    var mGraphGroup:RJTGraphGroup?
    var mRadio:RJTRadio?
    weak var viewController:iPhoneViewController?
    
    @IBInspectable var expandRemainingGraphs:Bool = false
        {
        didSet
        {
            mGraphGroup?.graphsShouldExpand(expandRemainingGraphs)
            setNeedsDisplay()
        }
    }
    
    var numGraphs:Int = 3
    {
        didSet
        {
            setNeedsDisplay()
        }
    }
    
    required init?(coder aDecoder: NSCoder)
    {
        super.init(coder: aDecoder)
    }
    
    override init(frame: CGRect)
    {
        super.init(frame: frame)
    }
    
    // Implement RJTViewUpdateProtocol
    func redraw()
    {
        setNeedsDisplay()
    }
    
    func updateGraphGroup()
    {
        mRadio = RJTRadio(txFreq: 18E3, rxFreq: 18E3)
        
        mRadio?.mReceiveDelegate = self

        mGraphGroup = RJTGraphGroup(numGraphs: Int32(numGraphs),
                                    frame:self.bounds,
                                    expandRemainingGraphs: expandRemainingGraphs)
        mGraphGroup?.delegate = self
        mGraphGroup?.loadDataSources(from: mRadio)
        setNeedsDisplay()
    }
    
    
    override func draw(_ rect: CGRect)
    {
        super.draw(rect)
        guard let ctx = UIGraphicsGetCurrentContext() else {return}
        
        mGraphGroup?.draw(ctx)
    }
    
    
    // Implement RJTRadioReceiveProtocol
    func receivedData(_ data: UnsafeMutablePointer<UInt8>!, dataLen len: UInt8)
    {
        let rawData = Data(bytes: data, count: Int(len));
        
        if let stringMessage = String(data: rawData, encoding: .ascii)
        {
            if let vc = viewController
            {
                vc.mReceivedMessageLabel.text = stringMessage
            }
        }
    }
    
    func finishedStopping()
    {
        if let vc = viewController
        {
            vc.mStarted = false;
            vc.mStartButton.isEnabled = true;
            vc.mStopButton.isEnabled = false;
        }
    }
}
