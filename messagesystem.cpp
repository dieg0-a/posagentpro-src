#include "messagesystem.h"
#include "printer.hpp"

std::mutex GlobalState::state_mutex;
device_status GlobalState::printer_status;
std::queue<std::pair<std::string, print_job_type>> GlobalState::raw_job_queue;
std::queue<label_info> GlobalState::label_job_queue;

std::map<std::string, Printer *> GlobalState::printer_drivers;

Printer *GlobalState::label_printer_driver;

// std::vector<Printer *> GlobalState::printer_drivers;
Printer *GlobalState::selected_printer;
bool GlobalState::network_thread_running;
std::thread *GlobalState::network_thread;
us_listen_socket_t *GlobalState::http_server_socket;
std::atomic<uWS::Loop *> GlobalState::network_loop;

bool GlobalState::autosave = false;
int GlobalState::current_printer_index;
int GlobalState::http_proxy_port = 9069;
std::atomic<bool> GlobalState::demo_mode = false;

int GlobalState::demo_print_jobs = 1;

std::function<bool(const std::string &key, std::string &val)>
    GlobalState::str_from_settings;
std::function<bool(const std::string &key, int &val)>
    GlobalState::int_from_settings;
std::function<bool(const std::string &key, bool &val)>
    GlobalState::bool_from_settings;

std::function<bool(const std::string &key, const std::string &val)>
    GlobalState::str_to_settings;
std::function<bool(const std::string &key, int val)>
    GlobalState::int_to_settings;
std::function<bool(const std::string &key, bool val)>
    GlobalState::bool_to_settings;

jpeg *GlobalState::last_jpeg_receipt = nullptr;

#include "escpos.hpp"

#if defined(WIN32) || defined(_WIN32) ||                                       \
    defined(__WIN32) && !defined(__CYGWIN__)
PrinterWindowsSpooler GlobalState::winprint;
PrinterWindowsSpooler GlobalState::windows_label_printer = PrinterWindowsSpooler(true);
#endif

#ifdef __linux__
PrinterLinuxUSBRAW GlobalState::linux_usb_print;
PrinterThermalLinuxTCPIP GlobalState::linux_ip_print;
LabelPrinterLinuxUSBRAW GlobalState::linux_label_printer;
#endif

PrinterQt GlobalState::qt_printer;

GlobalState::GlobalState() {}

std::string GlobalState::getCurrentPrinterName() {
  state_mutex.lock();
  auto temp =
      (selected_printer == nullptr) ? "None" : selected_printer->getName();
  state_mutex.unlock();
  return temp;
}

/*
bool GlobalState::enqueueJPEGPrintJob(std::string &raw)
{
    bool success = false;
    state_mutex.lock();
    if (raw.size() >= 1)
    {
        raw_job_queue.emplace(raw);
        success = true;
    }
    state_mutex.unlock();
    return success;
}
*/

bool GlobalState::enqueuePrintJob(std::string &&rawdata, print_job_type type) {
  bool success = false;
  state_mutex.lock();
  if (rawdata.size() >= 1) {
    raw_job_queue.push(std::make_pair<std::string, print_job_type>(
        std::move(rawdata), print_job_type(type)));
    success = true;
  }
  state_mutex.unlock();
  return success;
}

bool GlobalState::enqueueLabelPrintJob(label_info info) {
  bool success = false;
  state_mutex.lock();
  label_job_queue.push(info);
  success = true;
  state_mutex.unlock();
  return success;
}

/*
bool GlobalState::enqueuePrintJob(std::string &rawdata, print_job_type type)
{
    bool success = false;
    state_mutex.lock();
    if (rawdata.size() >= 1)
    {
        raw_job_queue.push(std::pair<std::string, print_job_type>(rawdata,
type)); success = true;
    }
    state_mutex.unlock();
    return success;
}
*/

bool GlobalState::processQueue() {
  bool queue_empty = true;
  state_mutex.lock();
  if (selected_printer != nullptr) {
    while (!raw_job_queue.empty()) {
      if (raw_job_queue.front().second == JPEG) {
        queue_empty = false;
        if (last_jpeg_receipt != nullptr) {
          delete last_jpeg_receipt;
          last_jpeg_receipt = nullptr;
        }
        last_jpeg_receipt = new jpeg();
        if (last_jpeg_receipt->decode(raw_job_queue.front().first))
          selected_printer->printJPEG(*last_jpeg_receipt);
        else {
          delete last_jpeg_receipt;
          last_jpeg_receipt = nullptr;
        }
      }

      else if (raw_job_queue.front().second == CASHDRAWER) {
        selected_printer->openCashDrawer();
      }
      raw_job_queue.pop();
    }
    while (!label_job_queue.empty()) {
      GlobalState::label_printer_driver->printLabel(label_job_queue.front());
      label_job_queue.pop();
    }
  }
  state_mutex.unlock();
  return !queue_empty;
}

bool GlobalState::setLastReceiptImage(std::string &jpeg_data) {
  bool success = false;
  state_mutex.lock();
  if (last_jpeg_receipt != nullptr) {
    delete last_jpeg_receipt;
    last_jpeg_receipt = nullptr;
  }
  last_jpeg_receipt = new jpeg();
  if (!last_jpeg_receipt->decode(jpeg_data)) {
    delete last_jpeg_receipt;
    last_jpeg_receipt = nullptr;
  } else
    success = true;
  state_mutex.unlock();
  return success;
}

void GlobalState::loadSettings() {
  int proxy_port;
  if (fromSettings("http_proxy_port", proxy_port))
    http_proxy_port = proxy_port;
  for (auto &[name, p] : printer_drivers) {
    for (auto &[fname, f] : p->getFieldsByName()) {
      if (f->get_type() == STRING) {
        std::string field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setString(fname, field);
      } else if (f->get_type() == INTEGER) {
        int field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setInt(fname, field);
      } else if (f->get_type() == INTEGER_RANGE) {
        int field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setInt(fname, field);
      } else if (f->get_type() == COMBO_LIST_STRING) {
        std::string field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setString(fname, field);
      } else if (f->get_type() == BOOLEAN_FIELD) {
        int field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setInt(fname, field);
      }
    }
  }
  std::string printer_driver_name;
  if (str_from_settings("current_printer_driver", printer_driver_name)) {
    setCurrentPrinter(printer_driver_name);
  }
  if (label_printer_driver) {
    auto p = label_printer_driver;
    auto name = label_printer_driver->getName();
    for (auto &[fname, f] : label_printer_driver->getFieldsByName()) {
      if (f->get_type() == STRING) {
        std::string field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setString(fname, field);
      } else if (f->get_type() == INTEGER) {
        int field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setInt(fname, field);
      } else if (f->get_type() == INTEGER_RANGE) {
        int field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setInt(fname, field);
      } else if (f->get_type() == COMBO_LIST_STRING) {
        std::string field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setString(fname, field);
      } else if (f->get_type() == BOOLEAN_FIELD) {
        int field;
        if (fromSettings("printer_" + name + "_field_" + fname, field))
          p->setInt(fname, field);
      }
    }
  }
}

void GlobalState::registerEventHandler(std::string name,
                                       std::function<void()> callback) {}
