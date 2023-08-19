#ifndef COMPONENT_H
#define COMPONENT_H

#include "object.hpp"
#include <functional>
#include <map>
#include <string>

class Component {
private:

protected:
  Object *parent;

public:
  Object *object() {return parent;};
  Component(Object *_parent) : parent(_parent) {};
  Component() {parent = nullptr;};
};

#endif // COMPONENT_H