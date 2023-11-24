#ifndef WINDOWSPRINTER_H
#define WINDOWSPRINTER_H

#include "printer.hpp"



#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
class PrinterWindowsSpooler : public PrinterRaw {
private:
    std::string name;
public:
    PrinterWindowsSpooler(bool label = false);
    std::string getName() const {
        return name;
    };
    device_status updateAndGetStatus();
    bool send_raw(const std::string &buffer);
    static std::vector<std::string> enumeratePrinters();
    ~PrinterWindowsSpooler();
};
#endif


#endif // WINDOWSPRINTER_H
