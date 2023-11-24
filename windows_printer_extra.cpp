#include "messagesystem.h"
#include "windowsprinter.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

PrinterWindowsSpooler::PrinterWindowsSpooler(bool label)
{
    name = "Windows Printer";
    addField(new string_combo_list_field("name", "Printer", enumeratePrinters(), 0), 0);
    if (label)
    GlobalState::registerLabelPrinter(name, this);
    else
    GlobalState::registerPrinter("Windows Printer", this);
}

#endif
