//
//  SineWave.hpp
//  CoreGraphicsMac
//
//  Created by Robby Tong on 10/4/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#ifndef SineWave_hpp
#define SineWave_hpp

#include <stdio.h>
#include "DataSource.h"

class SineWave: public DataSource
{
public:
    SineWave(int period);
    void setPeriod(int period);
    size_t size();
    AFPoint get_data(size_t index);
    void next();
    AFPoint get_origin();
    AFPoint get_lengths();
    bool valid();
    /* return milliseconds */
    int get_updateInterval();
    const char * name();
    const char * xlabel();
    const char * ylabel();
private:
    int mPeriod;
    char mBuf[64];
    bool mIsValid;
};
#endif /* SineWave_hpp */
