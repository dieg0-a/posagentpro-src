#include "printerlist.h"
#include "printer.hpp"

PrinterList::PrinterList(QWidget *parent) : QListView(parent)
{

    /*
    for (auto s : PrinterWindowsSpooler::enumeratePrinters())
    {
        printer_list.append(QString(s.c_str()));
    }
    model = new QStringListModel(printer_list, this);
    setModel(model);*/

}

void PrinterList::updatePrinterList()
{

}


PrinterList::~PrinterList()
{
    if (model != nullptr) delete model;
}
