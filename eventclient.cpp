#include "eventclient.h"
#include "eventsystem.h"

EventClient EventClient::root;

void EventClient::subscribeToEvent(std::string eventname) {
  EventSystem::instance().registerEventClientHandler(id, eventname);
}

void EventClient::desuscribeToEvent(std::string &eventname) {
  EventSystem::instance().deregisterEventClientHandler(id, eventname);
}

void EventClient::desuscribeToAllEvents() {
 }

void EventClient::handleEvent(std::string &name, EventData &data) {
  try {
    object()->onEvent(data);
  } catch (int e) {
    return;
  }
}

EventClient::EventClient(Object *_parent) : Component(_parent) {
  id = EventSystem::instance().registerEventClient(this);
}

EventClient::~EventClient() {
  EventSystem::instance().deregisterEventClient(id);
}

