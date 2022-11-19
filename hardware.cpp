#include <map>
#include "printer.hpp"

namespace hardware {

    std::map<int, Printer*> printers;
    Printer *current_printer;

    device_status printer_status() {return current_printer ? current_printer->updateAndGetStatus() : DISCONNECTED;};

    bool InstallPrinter(Printer *p) {
//        if (printers.find(p->getName()) == printers.end())
//        {
            printers.emplace(3, p);
            if (printers.size() > 0) current_printer = printers.begin()->second;
            return true;
//        }
//        else return false;
    }
}
