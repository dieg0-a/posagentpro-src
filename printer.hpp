#include <string>
#ifdef __linux__
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <list>
#include <map>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "escpos.hpp"
#include "inputfield.hpp"
#include "jpeg.hpp"

#pragma once

enum device_status { CONNECTED, DISCONNECTED, NOT_FOUND };

std::string to_string(const device_status &d);

class Printer {
protected:
  std::string type = "normal";
  std::string name;
  std::map<std::string, input_field *> fields;
  std::multimap<int, input_field *> field_map;

  bool cash_drawer_supported = false;

public:
  virtual std::string getName() const { return name; };
  virtual device_status updateAndGetStatus();
  virtual bool openCashDrawer();
  virtual int getCashDrawerEnabled() const { return cash_drawer_supported; };

  std::list<std::string>
  enumeratePrinters(); // Will enumerate all the available printers, if they are
                       // enumerable

  void setName(const std::string &n) { name = n; };
  void setInt(std::string name, int i);
  void setString(std::string name, std::string s);
  std::string setIndex(std::string name, int index);

  std::map<std::string, input_field *> const &getFieldsByName() const;
  std::multimap<int, input_field *> const &getFieldsByOrder() const;

  void fieldSetOrder(std::string name, int order) {
    input_field *temp;
    for (auto i = field_map.begin(), last = field_map.end(); i != last;) {
      if (i->second->name() == name) {
        temp = i->second;
        i = field_map.erase(i);
        break;
      } else {
        ++i;
      }
    }
    field_map.insert(std::pair<int, input_field *>(order, temp));
  };

  void removeField(std::string name) {
    input_field *temp;
    fields.erase(name);
    for (auto i = field_map.begin(), last = field_map.end(); i != last;) {
      if (i->second->name() == name) {
        temp = i->second;
        i = field_map.erase(i);
        delete temp;
        break;
      } else {
        ++i;
      }
    }
  };

  void addField(input_field *field, int order)
  {
    fields.emplace(field->name(), field);
    field_map.emplace(std::make_pair(order, field));
  }

  input_field const *getField(std::string name) const { return fields.at(name); }

  //    int getInt(std::string name) const;
  //    std::string getString(std::string name) const;
  //    std::vector<std::string> &getListOfStrings(std::string name);
  //    std::vector<int> &getListOfInts(std::string name);

  virtual bool printJPEG(const std::string &s);
  virtual bool printJPEG(const jpeg &jpeg_object);

  virtual ~Printer(){};

  std::string getType() { return type; };
};

class PrinterRaw : public Printer {

protected:
  int lines_to_feed_end() const {
    return fields.at("lines_to_feed")->get_int();
  }
  bool paper_cut() const { return fields.at("paper_cut")->get_int(); };
  int pixel_max_width() const { return fields.at("max_width")->get_int(); };
  int gamma() const { return fields.at("gamma")->get_int(); };

  std::string protocol_type() const {
    return fields.at("protocol_type")->get_string();
  };
  escpos escpos_generator;
  bool openCashDrawer();

public:
  PrinterRaw();
  int getPixelWidth() const { return pixel_max_width(); };
  int getGamma() const { return gamma(); };
  int getFeedLines() const { return lines_to_feed_end(); };
  std::string getPrintStandard() const { return protocol_type(); };
  int getCashDrawerEnabled() const { return cash_drawer_supported; };
  int getCutterEnabled() const { return paper_cut(); };

  virtual bool send_raw(
      const std::string &buffer); // Send length bytes of data to the printer

  void setPixelWidth(int n) {
    if (n < 384)
      n = 384;
    if (n > 768)
      n = 768;
    fields.at("max_width")->set_int(n);
    escpos_generator.max_width = n;
  };

  void setGamma(int g) {
    if (g < 50)
      g = 50;
    if (g > 1200)
      g = 1200;
    fields.at("gamma")->set_int(g);
    escpos_generator.gamma = g;
  };

  void setFeedLines(int n) {
    if (n < 0)
      n = 0;
    if (n > 20)
      n = 20;
    fields.at("lines_to_feed")->set_int(n);
  };

  void setPrintStandard(bool escpos_toggled) {
    fields.at("protocol_type")->set_string(escpos_toggled ? "escpos" : "star");
    escpos_generator.protocol_type = escpos_toggled ? "escpos" : "star";
  };
  void setCashDrawerEnabled(bool state) { cash_drawer_supported = state; };
  void setCutterEnabled(int state) { fields.at("paper_cut")->set_int(state); };

  bool printJPEG(const std::string &s);
  bool printJPEG(const jpeg &jpeg_object);
};

class PrinterDummy : public Printer {
public:
  PrinterDummy() { name = "dummy"; };
  std::string getName() const { return name; };
  device_status updateAndGetStatus();
  ~PrinterDummy(){};
};
#ifdef __linux__

class PrinterLinuxUSBRAW : public PrinterRaw {
private:
  std::string device() { return fields.at("device")->get_string(); };

public:
  PrinterLinuxUSBRAW();
  PrinterLinuxUSBRAW(const char *device_name);
  std::string getName() const { return name; };
  device_status updateAndGetStatus();
  bool send_raw(const std::string &buffer);
  ~PrinterLinuxUSBRAW(){};
};

class PrinterThermalLinuxTCPIP : public PrinterRaw {
private:
  std::string address() { return fields.at("address")->get_string(); };

  int port() {
    int port = 0;
    try {
      port = std::stoi(fields.at("port")->get_string());
    } catch (std::invalid_argument const &ex) {
      port = 9100;
    } catch (std::out_of_range const &ex) {
      port = 9100;
    }
    return port;
  };

  int sock = 0, client_fd;
  struct sockaddr_in serv_addr;

public:
  bool connected = false;
  bool connectToPrinter();
  PrinterThermalLinuxTCPIP();
  PrinterThermalLinuxTCPIP(const std::string &address, uint _port);
  std::string getName() const { return name; };
  device_status updateAndGetStatus();
  bool send_raw(const std::string &buffer);
  ~PrinterThermalLinuxTCPIP(){};
};
#endif
