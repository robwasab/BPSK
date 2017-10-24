//
//  ViewController.swift
//  AudioFrequencyIphoneExclusive
//
//  Created by Robby Tong on 10/22/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

import UIKit

class iPhoneViewController: UIViewController, UITextFieldDelegate
{
    @IBOutlet weak var mGraphView: GraphView!
    @IBOutlet weak var mTextField: UITextField!
    @IBOutlet weak var mReceivedMessageLabel: UILabel!
    @IBOutlet weak var mStopButton: UIButton!
    @IBOutlet weak var mStartButton: UIButton!
    
    var mStarted = false;

    @IBAction func startButtonPressed(_ sender: Any)
    {
        mStarted = true
        mStartButton.isEnabled = false
        mStopButton.isEnabled = true
        mGraphView.mRadio?.start()
    }
    
    @IBAction func stopButtonPressed(_ sender: Any)
    {
        mGraphView.mRadio?.stop()
    }
    
    @IBAction func sendButtonPressed(_ sender: Any)
    {
        if mStarted
        {
            mGraphView.mRadio?.send(withObject: mTextField.text)
        }
        else
        {
            print("You must start the radio first")
        }
    }
    
    func textFieldShouldReturn(_ textField: UITextField) -> Bool
    {
        // Hide the keyboard
        textField.resignFirstResponder()
        return true
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.view.backgroundColor = UIColor.white
        mGraphView.viewController = self
        mGraphView.updateGraphGroup()
        mTextField.delegate = self
        // Do any additional setup after loading the view, typically from a nib.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}

