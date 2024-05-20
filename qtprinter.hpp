#include "printer.hpp"
#pragma once

#include "eventdata.h"

#include <QtPrintSupport/QPrinter>
#include <QPrintPreviewDialog>
#include <QPageSetupDialog>

class PrinterQt : public PrinterRaw, Object {
private:
  QPrinter *printer;
  EventClient eventclient;
  QPrintPreviewDialog *preview_dialog;
  QPageSetupDialog *page_setup_dialog;

public:
  PrinterQt();

  std::string getName() const { return name; };

  device_status updateAndGetStatus();

  bool send_raw(const std::string &buffer);

  bool printJPEG(const std::string &s);
  bool printJPEG(const jpeg &jpeg_object);
  void onEvent(EventData d);

  ~PrinterQt() {if (printer != nullptr) delete printer;};
};
