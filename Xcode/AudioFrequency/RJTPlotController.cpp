//
//  PlotController.cpp
//  AudioFrequency
//
//  Created by Robby Tong on 10/1/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#include "RJTPlotController.h"
#include <vector>

using namespace std;

RJTPlotController::RJTPlotController():
mDataSources()
{
}

RJTPlotController::~RJTPlotController()
{
}

void RJTPlotController::add_plot(DataSource * source)
{
    mDataSources.push_back(source);
}

void RJTPlotController::remove_plot(DataSource * source)
{
    vector<DataSource *>::iterator it = mDataSources.begin();
    
    for (; it != mDataSources.end(); it++)
    {
        if (source == *it)
        {
            mDataSources.erase(it);
            break;
        }
    }
}

void RJTPlotController::set_close_cb(OnCloseCallback cb, void * obj)
{
    mOnCloseCb = cb;
    mOnCloseCbObj = obj;
}

int RJTPlotController::run()
{
    return 0;
}
