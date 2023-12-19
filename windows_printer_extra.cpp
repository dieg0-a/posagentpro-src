#include "messagesystem.h"
#include "windowsprinter.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

PrinterWindowsSpooler::PrinterWindowsSpooler()
{
    name = "Windows Printer";
    addField(new string_combo_list_field("name", "Printer", enumeratePrinters(), 0), 0);
    GlobalState::registerPrinter("Windows Printer", this);

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
    addField(new boolean_field("cash_drawer", "Enable Cash Drawer", false), 6);
}
#endif
