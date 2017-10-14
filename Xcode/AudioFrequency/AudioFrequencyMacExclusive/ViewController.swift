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
    @IBOutlet weak var mStartButton: NSButton!
    @IBOutlet weak var mStopButton: NSButton!
    @IBOutlet weak var mTextField: NSTextField!
    @IBOutlet weak var mDataReceivedLabel: NSTextField!
    var mStarted:Bool = false
    
    @IBAction func startButtonClicked(_ sender: NSButton)
    {
        mStarted = true
        mStartButton.isEnabled = false
        mStopButton.isEnabled = true
        graphView.mRadio?.start()
    }
    
    @IBAction func stopButtonClicked(_ sender: NSButton)
    {
        graphView.mRadio?.stop()
    }
    
    @IBAction func textFieldEnterPressed(_ sender: NSTextField)
    {
        if mStarted
        {
            graphView.mRadio?.send(withObject: sender.stringValue)
        }
        else
        {
            print("You must start the radio first")
        }
    }
    
    override func viewDidLoad()
    {
        super.viewDidLoad()
        graphView.viewController = self
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
    
    func windowShouldClose(_ sender: Any) -> Bool
    {
        if mStarted
        {
            let dialog = NSAlert()
            dialog.messageText = "You must stop the radio before closing!"
            dialog.runModal()
            return false
        }
        else
        {
            return true
        }
    }
}

