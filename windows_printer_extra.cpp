#include "messagesystem.h"
#include "windowsprinter.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

PrinterWindowsRawSpooler::PrinterWindowsRawSpooler()
{
    name = "Windows Printer";
    fields.emplace(std::make_pair("Name", new combo_list_field(enumeratePrinters(), 0)));
    GlobalState::registerPrinter("Windows Printer", this);
}
#endif
