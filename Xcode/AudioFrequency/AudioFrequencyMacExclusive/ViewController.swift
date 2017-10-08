//
//  ViewController.swift
//  AudioFrequencyMacExclusive
//
//  Created by Robby Tong on 9/30/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

import Cocoa

class ViewController: NSViewController, NSWindowDelegate {

    @IBOutlet weak var graphView: GraphView!
    
    override func viewDidLoad()
    {
        super.viewDidLoad()
    }
    
    override func dismissViewController(_ viewController: NSViewController) {
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }

    func windowWillResize(sender: NSWindow, toSize frameSize: NSSize) -> NSSize
    {
        graphView.bounds.size = frameSize as CGSize
        return frameSize
    }
    
    



}

