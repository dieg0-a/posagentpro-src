#pragma once
#include <string>
#ifdef __linux__
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#endif


#include <stdio.h>
#include <string.h>
#include <list>
#include <map>
#include <vector>

#include "escpos.hpp"

enum device_status {CONNECTED, DISCONNECTED, NOT_FOUND};

std::string to_string(const device_status &d);

enum input_field_type {COMBO_LIST, STRING, NUMBER};

class input_field {
    input_field_type type;
    int num_val;
    std::string string_val;
    int current_item_index;
    std::vector<std::string> options;

public:
    input_field(const std::vector<std::string> &options, unsigned int index);
    input_field(int val);
    input_field(std::string string);

    std::vector<std::string> &get_combo();
    std::string get_combo_selected() const;
    std::string &get_combo_at(int index);
    int get_combo_index() const { return current_item_index; };
    int get_num() const;
    std::string get_string() const;
    input_field_type get_type() const;

    void set_num(int n){num_val = n;};
    void set_string(std::string s){string_val = s;};
    void set_combo(std::string &combo);
    void set_combo(int index);
};

class Printer {
protected:
    std::string name;
    std::map<std::string, input_field> fields;

    int pixel_max_width = 576; //For raster image printing
    bool cash_drawer_supported = true;
    bool paper_cut = true;
    int lines_to_feed_end = 10;
    printer_type protocol_type = ESCPOS;
    escpos escpos_generator;


public:
    virtual std::string getName() const { return name; };
    virtual device_status updateAndGetStatus();
    virtual bool send_raw(const std::string &buffer);  //Send length bytes of data to the printer
    bool openCashDrawer();

    std::list<std::string> enumeratePrinters(); //Will enumerate all the available printers, if they are enumerable

    void setName(const std::string &n) {name = n;};
    void setInt(std::string name, int i);
    void setString(std::string name, std::string s);
    void setCombo(std::string name, std::string s);

    std::map<std::string, input_field> &getFields();
    int getInt(std::string name) const;
    std::string getString(std::string name) const;
    std::vector<std::string> &getCombo(std::string name);

    void setPixelWidth(int n) { 
        if (n < 384) n = 384;
        if (n > 768) n = 768;
        pixel_max_width = n;
        escpos_generator.max_width = n; 
    };
    void setFeedLines(int n) {
        if (n < 0) n = 0;
        if (n > 20) n = 20;
        lines_to_feed_end = n;
    };
    void setPrintStandard(bool escpos_toggled){protocol_type = escpos_toggled ? ESCPOS : STAR;
                                               escpos_generator.protocol_type = escpos_toggled ? ESCPOS : STAR;};
    void setCashDrawerEnabled(bool state){cash_drawer_supported = state;};
    void setCutterEnabled(int state){paper_cut = state == 0 ? false : true;};

    int getPixelWidth() const {return pixel_max_width;};
    int getFeedLines() const {return lines_to_feed_end;};
    printer_type getPrintStandard() const {return protocol_type;};
    int getCashDrawerEnabled() const {return cash_drawer_supported;};
    int getCutterEnabled() const {return paper_cut;};

    bool printJPEG(const std::string &s);


    virtual ~Printer(){};
};

class PrinterDummy : public Printer{
public:
    PrinterDummy(){name = "dummy";};
    std::string getName() const {return name;};
    bool send_raw(const std::string &) {return true;};
    device_status updateAndGetStatus();
    ~PrinterDummy(){};
};
#ifdef __linux__

class PrinterLinuxUSBRAW : public Printer {
private:
    std::string device(){return fields.at("Device").get_string();};
public:
    PrinterLinuxUSBRAW();
    PrinterLinuxUSBRAW(const char *device_name);
    std::string getName() const {return name;};
    device_status updateAndGetStatus();
    bool send_raw(const std::string &buffer);
    ~PrinterLinuxUSBRAW(){};
};

class PrinterThermalLinuxTCPIP : public Printer {
private:
    std::string address;
    uint port;
    PrinterThermalLinuxTCPIP();

    int sock = 0, client_fd;
    struct sockaddr_in serv_addr;

public:
    bool connected = false;
    bool connectToPrinter();
    PrinterThermalLinuxTCPIP(const std::string &address, uint _port);
    std::string getName() const {return name;};
    device_status updateAndGetStatus();
    bool send_raw(const std::string &buffer);
    ~PrinterThermalLinuxTCPIP(){};
};
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
class PrinterWindowsSpooler : public Printer {
private:
    std::string name;
public:
    PrinterWindowsSpooler();
    std::string getName() const {
        return name;
    };
    device_status updateAndGetStatus();
    bool send_raw(const std::string &buffer);
    static std::vector<std::string> enumeratePrinters();
    ~PrinterWindowsSpooler();
};
#endif


