#ifndef INPUTFIELD_HPP
#define INPUTFIELD_HPP
#include <string>
#include <vector>
#ifdef __linux__
#include <climits>
#include <iostream>
#endif

#include "eventsystem.h"

enum input_field_type {
  STRING,
  INTEGER,
  INTEGER_RANGE,
  COMBO_LIST_STRING,
  BOOLEAN_FIELD,
  ACTION
};
extern std::vector<std::string> empty_str_list;
extern std::vector<int> empty_int_list;

class input_field : public Object {
private:
protected:
  std::string _name;
  std::string _display_name;
  input_field_type type;
  std::string tooltip;
  std::string widget_type =
      "base"; //// Type of widget, one of {lineedit, spinbox, slider}
  EventClient eventclient;
  input_field() : eventclient(EventClient(this)){};

public:
  int order = 0;
  void set_tooltip(const std::string &t) { tooltip = t; };
  std::string get_tooltip() { return tooltip; };

  virtual std::vector<std::string> &get_list_of_str() {
    return empty_str_list;
  };
  virtual std::vector<int> &get_list_of_int() { return empty_int_list; };

  virtual int get_int_at(int index) { return -1; };
  virtual std::string get_string_at(int index) { return "Undefined"; };

  virtual int get_index() const { return -1; };
  virtual int get_int() const { return -1; };
  virtual std::string get_string() const { return "Undefined"; };

  input_field_type get_type() const { return type; };
  std::string get_widget_type() const { return widget_type; };
  virtual void set_int(int n){};
  virtual void set_string(std::string s){};
  virtual void set_index(int index){};

  std::string name() { return _name; };
  std::string display_name() { return _display_name; };
  void set_name(std::string name) { _name = name; };

  input_field(std::string name, std::string display_name)
      : _name(name), _display_name(display_name),
        eventclient(EventClient(this)){

        };
};

class integer_field : public input_field {
  int int_val;

public:
  integer_field(std::string name, std::string display_name, int val)
      : input_field(name, display_name) {
    int_val = val;
    type = INTEGER;
  };
  void set_int(int n) { int_val = n; };
  int get_int() const { return int_val; };

  void set_string(std::string s) {
    try {
      int_val = std::stoi(s);
    } catch (int num) {
      std::cerr << "integer_field: set_string(): Invalid string while "
                   "attempting to convert to integer!\n";
    }
  }
  std::string get_string() { return std::to_string(int_val); }
};

class string_field : public input_field {
  std::string string_val;

public:
  void set_string(std::string s) { string_val = s; };
  std::string get_string() const { return string_val; };

  void set_int(int i) {
    string_val = std::to_string(i);
    EventSystem::instance().emitEvent(
        _name + "_changed",
        EventData(_name + "_changed", i, eventclient.getID()));
  };
  int get_int() {
    int i = -1;
    try {
      i = std::stoi(string_val);

    } catch (int error) {
      std::cerr << "string_field: get_int(): Invalid string while attempting "
                   "to convert to integer!\n";
    }
    return i;
  };

  string_field(const std::string &name, const std::string &display_name,
               const std::string &string)
      : input_field(name, display_name) {
    string_val = string;
    type = STRING;
  };
};

class integer_range_field : public input_field {
  int int_val;
  int max, min;

public:
  integer_range_field(const std::string &name, const std::string &display_name,
                      int val, int _min = INT_MIN, int _max = INT_MAX,
                      std::string _widget_type = "lineedit")
      : input_field(name, display_name) {
    int_val = val;
    min = _min;
    max = _max;
    type = INTEGER_RANGE;
    if (_widget_type == "lineedit" || _widget_type == "spinbox" ||
        _widget_type == "slider")
      widget_type = _widget_type;
    else
      widget_type = "lineedit";
  };
  std::pair<int, int> getRange() { return std::make_pair(min, max); };
  void setRange(int _min, int _max) {
    min = _min;
    max = _max;
  };
  void set_int(int n) {
    if (n >= min && n <= max) {
      int_val = n;
      EventSystem::instance().emitEvent(
          _name + "_changed",
          EventData(_name + "_changed", n, eventclient.getID()));
    }
  };
  int get_int() const { return int_val; };

  int get_max() const { return max; };
  int get_min() const { return min; };

  void set_string(std::string s) {
    try {
      int j = std::stoi(s);
      if (j >= min && j <= max) {
        int_val = j;
        EventSystem::instance().emitEvent(
            _name + "_changed",
            EventData(_name + "_changed", int_val, eventclient.getID()));
      }

    } catch (int error) {
      std::cerr << "integer_range_field: set_string(): Invalid string while "
                   "attempting to convert to integer!\n";
    }
  }
};

class boolean_field : public input_field {
  bool value;

public:
  boolean_field(const std::string &name, const std::string &display_name,
                bool val, std::string _widget_type = "checkbox")
      : input_field(name, display_name) {
    type = BOOLEAN_FIELD;
    widget_type = _widget_type;
  };
  void set_int(int n) {

    value = n;
    EventSystem::instance().emitEvent(
        _name + "_changed",
        EventData(_name + "_changed", value, eventclient.getID()));
  };
  int get_int() const { return value; };

  void set_string(std::string s) {
    try {
      int j = std::stoi(s);
      value = j;
      EventSystem::instance().emitEvent(
          _name + "_changed",
          EventData(_name + "_changed", value, eventclient.getID()));

    } catch (int error) {
      std::cerr << "integer_range_field: set_string(): Invalid string while "
                   "attempting to convert to integer!\n";
    }
  }
};

class string_combo_list_field : public input_field {
private:
  std::vector<std::string> options;
  int current_item_index;

public:
  string_combo_list_field(const std::string &name,
                          const std::string &display_name,
                          const std::vector<std::string> &&o,
                          unsigned int index)
      : input_field(name, display_name) {
    options = o;
    type = COMBO_LIST_STRING;
    current_item_index = index < options.size() ? index : 0;
  };
  int get_index() const { return current_item_index; };

  std::vector<std::string> &get_list_of_str() { return options; };
  std::string get_string() const { return options.at(current_item_index); };
  std::string get_string_at(int index) {
    if (index < options.size())
      return options.at(index);
    else
      return "Undefined";
  };

  void set_string(std::string combo) {
    if (options.empty())
      return;
    if (combo == options[current_item_index])
      return;
    for (int i = 0; i < options.size(); i++) {
      if (options[i] == combo) {
        current_item_index = i;
        EventSystem::instance().emitEvent(
            _name + "_changed",
            EventData(_name + "_changed", combo, eventclient.getID()));
            break;
      }
    }
  };
  void set_index(int index) {
    if (index < options.size())
      if (index != current_item_index) {
        current_item_index = index;
        EventSystem::instance().emitEvent(_name + "_changed",
                                          EventData(_name + "_changed",
                                                    options[current_item_index],
                                                    eventclient.getID()));
      }
  };
};

class action_field : public input_field {
private:
public:
  action_field(const std::string &name, const std::string &display_name)
      : input_field(name, display_name) {
    type = ACTION;
  };

  void executeAction() {
    EventSystem::instance().emitEvent(
        _name + "_executed",
        EventData(_name + "_executed", eventclient.getID()));
  };
};

#endif // INPUTFIELD_HPP
