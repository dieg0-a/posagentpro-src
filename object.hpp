#ifndef OBJECT_H
#define OBJECT_H

#include <functional>
#include <map>
#include <string>
#include "eventdata.h"

class Object {
public:
    virtual void onEvent(EventData d) {
        return;
    }



};



#endif // OBJECT_H