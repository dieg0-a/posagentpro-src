#ifndef EVENT_CLIENT_H
#define EVENT_CLIENT_H

#include "eventdata.h"
#include "component.hpp"


#include <functional>
#include <map>
#include <string>


class EventClient : public Component {
private:
  int id;
  EventClient(){
    id = 0;    
  };

public:
  void subscribeToEvent(std::string eventname);
  void desuscribeToEvent(std::string &eventname);
  void desuscribeToAllEvents();
  void handleEvent(std::string &name, EventData &data);
  int getID() {return id;};

  static EventClient root;

  EventClient(Object *_parent);
  ~EventClient();
};

#endif // EVENT_CLIENT_H