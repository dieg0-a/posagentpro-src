#include "messagesystem.h"
#include "windowsprinter.h"

PrinterWindowsSpooler::PrinterWindowsSpooler()
{
    name = "Windows Printer";
    fields.emplace(std::make_pair("Name", new combo_list_field(enumeratePrinters(), 0)));
    GlobalState::registerPrinter("Windows Printer", this);
}
