#include "qtprinter.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <qnamespace.h>
#include <string>

#include <math.h>

#include "inputfield.hpp"
#include "messagesystem.h"

#include <QPageSetupDialog>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrinterInfo>

extern int bayer_matrix[];

inline double bayer(int i, int j) {
  return double(bayer_matrix[i * 4 + j]) / 64.0;
}

inline double fastPow(double a, double b) {
  union {
    double d;
    int x[2];
  } u = {a};
  u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}

device_status PrinterQt::updateAndGetStatus() {
  //    return std::filesystem::exists(device()) ? CONNECTED : DISCONNECTED;
  return CONNECTED;
}

PrinterQt::PrinterQt() : eventclient(EventClient(this)) {
  name = "System Printer";

  fields.erase("lines_to_feed");
  fields.erase("paper_cut");
  fields.erase("max_width");
  fields.erase("protocol_type");

  fields.emplace(
      std::make_pair("settings", new action_field("settings", "Settings...")));
  fields.emplace(std::make_pair(
      "pagesetup", new action_field("pagesetup", "Page Setup...")));

  auto qtprinters = QPrinterInfo::availablePrinterNames();

  std::vector<std::string> printers;

  for (auto p : qtprinters) {
    printers.push_back(p.toStdString());
  }

  fields.emplace(std::make_pair(
      "printer", new string_combo_list_field("printer", "Printer",
                                             std::move(printers), 0)));

  GlobalState::registerPrinter(name, this);
  eventclient.subscribeToEvent("settings_executed");
  eventclient.subscribeToEvent("pagesetup_executed");
  eventclient.subscribeToEvent("qapplication_ready");
  eventclient.subscribeToEvent("printer_changed");
};

bool PrinterQt::send_raw(const std::string &buffer) { return true; }

#include "mainwindow.h"

void PrinterQt::onEvent(EventData d) {
  if (d.name() == "settings_executed") {
    if (preview_dialog != nullptr) {
      delete preview_dialog;
    }

    preview_dialog = new QPrintPreviewDialog(
        printer, MainWindow::active_window, Qt::WindowFlags::enum_type::Dialog);
    QObject::connect(preview_dialog, SIGNAL(paintRequested(QPrinter *)),
                     MainWindow::active_window,
                     SLOT(paintPrintPreview(QPrinter *)));
    preview_dialog->setWindowTitle(("Print Document"));

    preview_dialog->show();
  } else if (d.name() == "pagesetup_executed") {
    if (page_setup_dialog != nullptr) {
      delete page_setup_dialog;
    }
    page_setup_dialog =
        new QPageSetupDialog(printer, MainWindow::active_window);
    page_setup_dialog->setWindowTitle(("Page Setup"));
    page_setup_dialog->show();
  } else if (d.name() == "qapplication_ready") {
    printer = new QPrinter(QPrinter::HighResolution);
    printer->setOutputFormat(QPrinter::OutputFormat::PdfFormat);
  } else if (d.name() == "printer_changed") { /*
     if (printer != nullptr)
       delete printer;
     std::cout << "DEBUG: SELECTED PRINTER HAS NAME: "
               << fields.at("printer")->get_string().c_str() << "\n";
     printer = new QPrinter(
         QPrinterInfo::printerInfo(fields.at("printer")->get_string().c_str()));
     printer->setOutputFormat(QPrinter::OutputFormat::PdfFormat);
     */
  }
}

bool PrinterQt::printJPEG(const std::string &s) { return true; }
bool PrinterQt::printJPEG(const jpeg &jpeg_object) {

  int max_width = 512;
  std::stringstream receipt_buf;
  receipt_buf.str(std::string());
  auto s = jpeg_object.image_memory_ptr();
  int height = jpeg_object.height();
  int width = jpeg_object.width();
  int bytespp = jpeg_object.components();
  double ratio = double(width) / double(max_width);
  int new_height = height / ratio;

  double gamma_d = (double)fields.at("gamma")->get_int();

  short bitcounter = 0;
  unsigned char buffer_byte = '\0';
  for (int i = 0; i < new_height; i++) {
    for (int j = 0; j < (max_width); j++) {

      double i_s = double(i) * ratio;
      double j_s = double(j) * ratio;

      double i_bottom_d = floor(i_s);
      double i_top_d = ceil(i_s) <= height - 1 ? ceil(i_s) : height - 1;
      double j_bottom_d = floor(j_s);
      double j_top_d = ceil(j_s) <= width - 1 ? ceil(j_s) : width - 1;

      int i_bottom = i_bottom_d;
      int i_top = i_top_d;
      int j_bottom = j_bottom_d;
      int j_top = j_top_d;

      double dx = i_s - i_bottom_d;
      double dy = j_s - j_bottom_d;

      int colorbyte = 0;

      for (int k = 0; k < bytespp; k++) {
        double c_bottom_left = s[i_bottom][(j_bottom * bytespp) + k];
        double c_bottom_right = s[i_bottom][(j_top * bytespp) + k];
        double c_top_left = s[i_top][(j_bottom * bytespp) + k];
        double c_top_right = s[i_top][(j_top * bytespp) + k];

        double dcdytop = c_top_right - c_top_left;
        double dcdybottom = c_bottom_right - c_bottom_left;
        double c_top = c_top_left + dcdytop * dy;
        double c_bottom = c_bottom_left + dcdybottom * dy;

        double dcdx = c_top - c_bottom;

        double component = c_bottom + dcdx * dx;
        colorbyte += component;
      }
      colorbyte /= bytespp;

      colorbyte = colorbyte <= 0xFF ? colorbyte : 0xFF;
      // Transform to linear RGB

      double color = double(colorbyte) / 255.0;

      if (color < 0.04045)
        color = color / 12.92;
      else
        color = fastPow((color + 0.055) / 1.055, gamma_d / 100.0);

      int colorbyte_linear = color * 255.0;
      if (colorbyte_linear < 0xAA) {
        if (colorbyte_linear > 0x60) {
          if (bayer(i % 4, j % 4) * 255 < colorbyte_linear) {
            buffer_byte += (unsigned char)(0x01 << (7 - bitcounter));
          }
        }
      } else
        buffer_byte += (unsigned char)(0x01 << (7 - bitcounter));

      bitcounter++;
      if (bitcounter == 8) {
        bitcounter = 0;
        receipt_buf << buffer_byte;
        buffer_byte = 0;
      }
    }
  }

  QImage i = QImage((unsigned char *)receipt_buf.str().c_str(), max_width,
                    new_height, QImage::Format::Format_Mono);

  QPainter painter;


////
  printer->setOutputFileName("file.pdf");
////


  painter.begin(printer);
  auto margins = printer->pageLayout().marginsPixels(printer->resolution());
  QRectF pagerect =
      printer->pageLayout().paintRectPixels(printer->resolution());
  QRectF source_rect_float = QRectF(0, 0, max_width, new_height);

  QRectF printable_area_pixels = QRectF(
      0, 0, pagerect.width(),
      ((float)(pagerect.width()) / (float)max_width) * (float)new_height);
  painter.drawImage(printable_area_pixels, i,
                    QRect(0, 0, max_width, new_height));
  // Use the painter to draw on the page.
  painter.end();
  return true;
}
