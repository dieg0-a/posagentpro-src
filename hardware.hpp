#include "printer.hpp"
#include <map>
#pragma once

namespace hardware {
    extern std::map<int,Printer*> printers;
    extern Printer *current_printer;

    device_status printer_status();

    bool InstallPrinter(Printer *p);

}
