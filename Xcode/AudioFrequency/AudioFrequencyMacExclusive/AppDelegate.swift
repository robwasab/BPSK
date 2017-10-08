//
//  AppDelegate.swift
//  AudioFrequencyMacExclusive
//
//  Created by Robby Tong on 9/30/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {



    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
        if let window = NSApplication.shared().keyWindow
        {
            if let viewController = window.contentViewController as? ViewController
            {
                window.delegate = viewController
            }
        }
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
       
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }

}

