#include "printer.hpp"
#include "inputfield.hpp"
#include "messagesystem.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

std::string to_string(const device_status &d) {
  return d == CONNECTED ? "connected" : "disconnected";
};

device_status Printer::updateAndGetStatus() { return DISCONNECTED; }

bool Printer::printJPEG(const std::string &s) { return true; };
bool Printer::printJPEG(const jpeg &jpeg_object) { return true; };

device_status PrinterDummy::updateAndGetStatus() { return CONNECTED; };

void Printer::setInt(std::string name, int i) {
  auto s = fields.find(name);
  if (s != fields.end())
    s->second->set_int(i);
  else {
    std::cout << "DEBUG: WARNING REQUESTING A NON EXISTENT FIELD\n";
  }
}

void Printer::setString(std::string name, std::string str) {
  auto s = fields.find(name);
  if (s != fields.end())
    s->second->set_string(str);
  else {
    std::cout << "DEBUG: WARNING REQUESTING A NON EXISTENT FIELD\n";
  }
}

std::string Printer::setIndex(std::string name, int index) {
  auto s = fields.find(name);
  if (s != fields.end()) {
    s->second->set_index(index);
    return s->second->get_string();
  } else {
    std::cout << "DEBUG: WARNING REQUESTING A NON EXISTENT FIELD\n";
    return "";
  }
}

std::map<std::string, input_field *> const &Printer::getFieldsByName() const {
  return fields;
}

std::multimap<int, input_field *> const &Printer::getFieldsByOrder() const {
  return field_map;
}

/*
int Printer::getInt(std::string name) const
{
    auto s = fields.find(name);
    if (s != fields.end()) return s->second->get_int();
    else
    {
        std::cout << "DEBUG: WARNING REQUESTING A NON EXISTENT FIELD\n";
        return -1;
    }
}

std::string Printer::getString(std::string name) const
{
    auto s = fields.find(name);
    if (s != fields.end()) return s->second->get_string();
    else
    {
        std::cout << "DEBUG: WARNING REQUESTING A NON EXISTENT FIELD\n";
        return "";
    }
}

std::vector<std::string> empty_str_vector;

std::vector<std::string> &Printer::getListOfStrings(std::string name)
{
    auto s = fields.find(name);
    if (s != fields.end()) return s->second->get_list_of_str();
    else
    {
        std::cout << "DEBUG: WARNING REQUESTING A NON EXISTENT FIELD\n";
        return empty_str_vector;
    }
}
*/

bool Printer::openCashDrawer() { return cash_drawer_supported; }

bool PrinterRaw::send_raw(const std::string &) {
  return true;
} // Send length bytes of data to the printer

bool PrinterRaw::printJPEG(const std::string &s) {
  escpos_generator.begin().image_from_jpeg(s).feednlines(lines_to_feed_end());
  if (paper_cut())
    escpos_generator.fullcut();
  std::string output = escpos_generator.end();
  return send_raw(output);
}

bool PrinterRaw::printJPEG(const jpeg &jpeg_object) {
  escpos_generator.begin()
      .image_from_jpeg(jpeg_object)
      .feednlines(lines_to_feed_end());
  if (paper_cut())
    escpos_generator.fullcut();
  std::string output = escpos_generator.end();
  return send_raw(output);
}

bool PrinterRaw::openCashDrawer() {
  return (cash_drawer_supported)
             ? send_raw(escpos_generator.begin().cashdrawer().end())
             : true;
}

PrinterRaw::PrinterRaw() {
  name = "Abstract RAW Printer";
  type = "raw";
  //    fields.emplace(std::make_pair("protocol_type", new
  //    string_combo_list_field(std::vector<std::string, int>())));
}

#ifdef __linux__

PrinterLinuxUSBRAW::PrinterLinuxUSBRAW() : PrinterRaw() {
  name = "Linux USB Printer";
  addField(new string_field("device", "Device", "/dev/usb/lp0"), 0);
  std::vector<std::string> protocol_type;
  protocol_type.emplace(protocol_type.end(), std::string("escpos"));
  protocol_type.emplace(protocol_type.end(), std::string("star"));

  addField(new string_combo_list_field("protocol_type", "Protocol",
                                       std::move(protocol_type), 0),
           1);
  addField(new integer_range_field("gamma", "Gamma", 1000, 200, 1000, "slider"),
           2);
  addField(new integer_range_field("max_width", "Max Width", 576, 384, 576,
                                   "slider"),
           3);
  addField(new integer_range_field("lines_to_feed", "Feed Lines", 0, 0, 20,
                                   "spinbox"),
           4);
  addField(new boolean_field("paper_cut", "Cut Paper", false), 5);

  GlobalState::registerPrinter(name, this);
}

device_status PrinterLinuxUSBRAW::updateAndGetStatus() {
  return std::filesystem::exists(device()) ? CONNECTED : DISCONNECTED;
}

bool PrinterLinuxUSBRAW::send_raw(const std::string &buffer) {
  std::fstream file;
  try {
    file.exceptions(std::ofstream::badbit | std::ofstream::failbit);
    file.open(fields.at("Device")->get_string(), std::ios::in | std::ios::out);
    file << buffer;
  } catch (const std::ofstream::failure &e) {
    std::cout << "Failure to open or write to Linux USB Raw Printer! Check "
                 "permissions and address...\n";
    if (file.is_open())
      file.close();
    return false;
  }
  if (file.is_open())
    file.close();
  return true;
}

PrinterThermalLinuxTCPIP::PrinterThermalLinuxTCPIP() {
  name = "Linux TCP/IP Printer";
  addField(new string_field("address", "Address", "127.0.0.1"), 0);
  addField(new string_field("port", "Port", std::to_string(9100)), 1);
  std::vector<std::string> protocol_type;
  protocol_type.emplace(protocol_type.end(), std::string("escpos"));
  protocol_type.emplace(protocol_type.end(), std::string("star"));

  addField(new string_combo_list_field("protocol_type", "Protocol",
                                       std::move(protocol_type), 0),
           2);
  addField(new integer_range_field("gamma", "Gamma", 1000, 200, 1000, "slider"),
           3);
  addField(new integer_range_field("max_width", "Max Width", 576, 384, 576,
                                   "slider"),
           4);
  addField(new integer_range_field("lines_to_feed", "Feed Lines", 0, 0, 20,
                                   "spinbox"),
           5);
  addField(new boolean_field("paper_cut", "Cut Paper", false), 6);

  GlobalState::registerPrinter(name, this);
}

PrinterThermalLinuxTCPIP::PrinterThermalLinuxTCPIP(const std::string &_address,
                                                   uint _port) {
  addField(new string_field("address", "Address", _address), 0);
  addField(new string_field("port", "Port", std::to_string(_port)), 1);
  std::vector<std::string> protocol_type;
  protocol_type.emplace(protocol_type.end(), std::string("escpos"));
  protocol_type.emplace(protocol_type.end(), std::string("star"));

  addField(new string_combo_list_field("protocol_type", "Protocol",
                                       std::move(protocol_type), 0),
           2);
  addField(new integer_range_field("gamma", "Gamma", 1000, 200, 1000, "slider"),
           3);
  addField(new integer_range_field("max_width", "Max Width", 576, 384, 576,
                                   "slider"),
           4);
  addField(new integer_range_field("lines_to_feed", "Feed Lines", 0, 0, 20,
                                   "spinbox"),
           5);
  addField(new boolean_field("paper_cut", "Cut Paper", false), 6);
}

bool PrinterThermalLinuxTCPIP::connectToPrinter() {
  if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    printf("\n Socket creation error \n");
  }
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port());

  // Convert IPv4 and IPv6 addresses from text to binary
  // form
  if (inet_pton(AF_INET, address().c_str(), &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return false;
  }

  if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
                           sizeof(serv_addr))) < 0) {
    printf("\nConnection Failed \n");
    return false;
  }
  connected = true;
  return true;
}

bool PrinterThermalLinuxTCPIP::send_raw(const std::string &buffer) {
  std::cout << "Called send_raw\n";

  if (!connected) {
    if (!connectToPrinter()) {
      return false;
    }
  }
  if (send(sock, buffer.c_str(), buffer.length(), 0) == -1) {
    connected = false;
    return false;
  }
  if (send(sock, (const char *)EOF, 1, 0) == -1) {
    connected = false;
    return false;
  }
  //    printf("Hello message sent\n");
  //      char b[1024];
  //      read(sock, b, 1024);
  //    printf("%s\n", buffer);

  // closing the connected socket
  /*
  char discard[100];
  while (read(client_fd, discard, sizeof discard) > 0);
  while (shutdown(client_fd, 1) != 0)
  {


  }
  while (close(client_fd) != 0){};
  connected = false;
  */
  return true;
}

device_status PrinterThermalLinuxTCPIP::updateAndGetStatus() {
  return CONNECTED;
}

#endif
