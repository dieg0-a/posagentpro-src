#ifndef WINDOWSPRINTER_H
#define WINDOWSPRINTER_H

#include "printer.hpp"

#include <windows.h>


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
class PrinterWindowsSpooler : public PrinterRaw {
private:
    std::string name;
    bool __send_raw(unsigned char* lpData, DWORD dwCount);
public:
    PrinterWindowsSpooler();
    std::string getName() const {
        return name;
    };
    device_status updateAndGetStatus();
    bool send_raw(const std::string &buffer);
    bool send_raw(const std::vector<unsigned char> buffer);
    static std::vector<std::string> enumeratePrinters();
    ~PrinterWindowsSpooler();
};
#endif


#endif // WINDOWSPRINTER_H
