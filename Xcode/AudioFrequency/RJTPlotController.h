//
//  RJTPlotController.h
//  AudioFrequency
//
//  Created by Robby Tong on 10/8/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#ifndef RJTPlotController_h
#define RJTPlotController_h

#include "PlotController.h"
#include <vector>

using namespace std;

/* This class implements the PlotController interface in such a way that
 * it can be conveniently used with Objective C.
 */

class RJTPlotController: public PlotController
{
public:
    RJTPlotController();
    ~RJTPlotController();
    
    /* Inherited */
    void add_plot(DataSource * source);
    void remove_plot(DataSource * source);
    
    /* The transceiver will set cb to a function that shuts it down.
     * Therefore, the PlotController must call cb when the GUI window is closed.
     */
    void set_close_cb(OnCloseCallback cb, void * obj);
    
    /* Unimplemented in this version, since the main run loop is the OSX's run loop.
     * In QT, you must call a main run loop function that blocks until the user presses the close window button.
     */
    int run();
    
    /* This is where it keeps its array of data sources.
     * RJTRadio.mm will use this instance variable to retreive the sources
     */
    vector<DataSource *> mDataSources;
private:
    OnCloseCallback mOnCloseCb = NULL;
    void * mOnCloseCbObj = NULL;
};

#endif /* RJTPlotController_h */
