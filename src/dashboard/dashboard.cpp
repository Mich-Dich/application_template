
#include "util/pch.h"

#include "events/event.h"
#include "events/application_event.h"
#include "events/mouse_event.h"
#include "events/key_event.h"

#include "dashboard.h"


namespace AT {

    dashboard::dashboard() {}
    
    
    dashboard::~dashboard() {}


    // init will be called when every system is initalized
    bool dashboard::init() {

        return true;
    }

    // shutdown will be called bevor any system is deinitalized
    bool dashboard::shutdown() {

        return true;
    }


    void dashboard::update(f32 delta_time) {}


    void dashboard::draw(f32 delta_time) {}


    void dashboard::on_event(event& event) {}

}
