//
//  SineWave.cpp
//  CoreGraphicsMac
//
//  Created by Robby Tong on 10/4/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#include "SineWave.hpp"
#include <math.h>
#include <string.h>

SineWave::SineWave(int period):
    DataSource()
{
    mPeriod = period;
    mIsValid = false;
}

void SineWave::setPeriod(int period)
{
    mPeriod = period;
}

size_t SineWave::size()
{
    return 256;
}

AFPoint SineWave::get_data(size_t index)
{
    double x = index / 256.0 * mPeriod * 2.0 * M_PI;
    double y = sin(x);
    AFPoint ret;
    ret.x = x / M_PI;
    ret.y = y;
    return ret;
}

void SineWave::next()
{
    mIsValid = true;
}

AFPoint SineWave::get_origin()
{
    AFPoint pnt;
    pnt.x = 0;
    pnt.y = -1.5;
    return pnt;
}

AFPoint SineWave::get_lengths()
{
    AFPoint dimension;
    dimension.x = mPeriod * 2.0;
    dimension.y = 3.0;
    return dimension;
}

bool SineWave::valid()
{
    return mIsValid;
}

/* return milliseconds */
int SineWave::get_updateInterval()
{
    return 1000;
}

const char * SineWave::name()
{
    memset(mBuf, 0, sizeof(mBuf));
    sprintf(mBuf, "SineWave %d Hz", mPeriod);
    return mBuf;
}

const char * SineWave::xlabel()
{
    static const char label[] = "Index";
    return label;
}

const char * SineWave::ylabel()
{
    static const char label[] = "Amplitude";
    return label;
}
