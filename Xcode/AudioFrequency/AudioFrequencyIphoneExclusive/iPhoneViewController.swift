//
//  ViewController.swift
//  AudioFrequencyIphoneExclusive
//
//  Created by Robby Tong on 10/22/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

import UIKit

class ViewController: UIViewController
{
    @IBOutlet weak var mGraphView: GraphView!
    @IBOutlet weak var mTextField: UITextField!
    @IBOutlet weak var mReceivedMessageLabel: UILabel!

    @IBAction func startButtonPressed(_ sender: Any)
    {
    }
    
    @IBAction func stopButtonPressed(_ sender: Any)
    {
    }
    
    @IBAction func sendButtonPressed(_ sender: Any)
    {
    }
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.view.backgroundColor = UIColor.white
        mGraphView.updateGraphGroup()
        
        // Do any additional setup after loading the view, typically from a nib.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}

