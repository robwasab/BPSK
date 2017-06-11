#include "PlotController.h"
#include "DataSource.h"
#include <math.h>
#include <stdio.h>

class TestSource: public DataSource
{
public:
    TestSource(size_t len = 1024) : len(len), carrier_inc(2.0*M_PI*(1.0/len)), modulat_inc(2.0*M_PI*(8.0/len))
    {
        modulat_phase = 0;
        carrier_phase = 0;
    }
    size_t size() {
        return len;
    }
    Point get_data(size_t index) {
        Point pnt;
        pnt.x = index;
        pnt.y = sin(modulat_phase - index * modulat_inc)*sin(carrier_phase - index * carrier_inc);
        return pnt;
    }
    void next() {
        modulat_phase += 32*modulat_inc;
        carrier_phase += 32*carrier_inc;
    }

    Point get_origin() {
        Point pnt;
        pnt.x = 0;
        pnt.y = -1.5;
        return pnt;
    }

    Point get_lengths() {
        Point pnt;
        pnt.x = len;
        pnt.y = 3.0;
        return pnt;
    }

    bool valid()
    {
        return false;
    }

private:
    void * plot;
    const size_t len;
    const float carrier_inc;
    const float modulat_inc;
    double carrier_phase;
    double modulat_phase;
};

int main(int argc, char ** argv)
{
    TestSource * sources = new TestSource[3];
    PlotController controller(argc, argv);
    controller.add_plot(&sources[0]);
    controller.add_plot(&sources[1]);
    controller.add_plot(&sources[2]);
    controller.remove_plot(&sources[0]);
    controller.remove_plot(&sources[1]);
    controller.add_plot(&sources[0]);
    controller.add_plot(&sources[1]);
    return controller.run();
}
