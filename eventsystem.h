#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <functional>
#include <list>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>

#include "eventclient.h"

class EventSystem {
private:
  static EventSystem *_instance;
  static int clientcount;
  std::map<int, EventClient *> clients;

  std::map<std::string, std::list<int>> event_handlers;

public:
  static EventSystem &instance() {
    if (_instance == nullptr)
      _instance = new EventSystem;
    return *_instance;
  }

  int registerEventClient(EventClient *client) {
    while (clients.contains(clientcount))
      clientcount++;
    clients.emplace(std::pair<int, EventClient *>(clientcount, client));
    return clientcount;
  }

  void registerEventClientHandler(int id, std::string eventname) {
    if (!event_handlers.contains(eventname)) {
      std::list<int> new_event;
      new_event.push_back(id);
      event_handlers.emplace(
          std::pair<std::string, std::list<int>>(eventname, new_event));
    } else {
      event_handlers[eventname].push_back(id);
    }
  }

  void deregisterEventClient(int client) {
    for (auto name : event_handlers) {
      name.second.remove(client);
    }
    std::list<std::string> toremove;
    for (auto name : event_handlers) {
        if (name.second.empty())
            toremove.push_back(name.first);
    }
    for (auto s : toremove)
    {
        event_handlers.erase(s);
    }
    
  }

  void deregisterEventClientHandler(int client, std::string eventname) {
    if (event_handlers.contains(eventname)) {
      try {
        event_handlers[eventname].remove(client);
      } catch (int e) {
      }
      if (event_handlers[eventname].empty())
        event_handlers.erase(eventname);
    }
  }

  void emitEvent(std::string name, EventData &data) {
    try {
      auto handlers = event_handlers.at(name);
      for (auto handler_id : handlers) {
        clients[handler_id]->handleEvent(name, data);
      }
    } catch (const std::out_of_range &e) {
      return;
    }
  }

  void emitEvent(std::string name, EventData &&data) {
    try {
      auto handlers = event_handlers.at(name);
      for (auto handler_id : handlers) {
        clients[handler_id]->handleEvent(name, data);
      }
    } catch (const std::out_of_range &e) {
      return;
    }
  }

  EventClient *clientByID(int id){
    auto it = clients.find(id);
    if (it != clients.end()){
      return it->second;
    }
    else return nullptr;
  }
  
};

#endif // EVENT_SYSTEM_H