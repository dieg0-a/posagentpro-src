#include "messagesystem.h"

std::mutex GlobalState::state_mutex;
device_status GlobalState::printer_status;
std::queue<std::pair<std::string, print_job_type>> GlobalState::raw_job_queue;
std::map<std::string, Printer*> GlobalState::printer_drivers;
Printer *GlobalState::selected_printer;
bool GlobalState::network_thread_running;
std::thread *GlobalState::network_thread;
us_listen_socket_t *GlobalState::http_server_socket;
bool GlobalState::autosave = false;
int GlobalState::current_printer_index;
int GlobalState::http_proxy_port = 9069;
std::atomic<bool> GlobalState::demo_mode = false;

int GlobalState::demo_print_jobs = 1;

std::function<bool(const std::string &key, std::string &val)> GlobalState::str_from_settings;
std::function<bool(const std::string &key, int &val)> GlobalState::int_from_settings;
std::function<bool(const std::string &key, bool &val)> GlobalState::bool_from_settings;

std::function<bool(const std::string &key, const std::string &val)> GlobalState::str_to_settings;
std::function<bool(const std::string &key, int val)> GlobalState::int_to_settings;
std::function<bool(const std::string &key, bool val)> GlobalState::bool_to_settings;


#include "escpos.hpp"


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    PrinterWindowsSpooler GlobalState::winprint;
#endif
#ifdef __linux__
    PrinterLinuxUSBRAW GlobalState::linux_usb_print;
#endif


GlobalState::GlobalState()
{

}

std::string GlobalState::getCurrentPrinterName()
{
    state_mutex.lock();
    auto temp = (selected_printer == nullptr) ? "None" : selected_printer->getName();
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

bool GlobalState::enqueuePrintJob(std::string &&rawdata, print_job_type type)
{
    bool success = false;
    state_mutex.lock();
    if (rawdata.size() >= 1)
    {
        raw_job_queue.push(std::make_pair<std::string, print_job_type>(std::move(rawdata), print_job_type(type)));
        success = true;
    }
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
        raw_job_queue.push(std::pair<std::string, print_job_type>(rawdata, type));
        success = true;
    }
    state_mutex.unlock();
    return success;
}
*/

void GlobalState::processQueue()
{
    state_mutex.lock();
    if (selected_printer != nullptr)
    {
        while (!raw_job_queue.empty())
        {
            if (raw_job_queue.front().second == JPEG)
                selected_printer->printJPEG(raw_job_queue.front().first);
            else if (raw_job_queue.front().second == CASHDRAWER)
            {
                selected_printer->openCashDrawer();
            }
            raw_job_queue.pop();
        }
    }
    state_mutex.unlock();
}

void GlobalState::loadSettings()
{
    int proxy_port;
    if (fromSettings("http_proxy_port", proxy_port))
        http_proxy_port = proxy_port;
    for (auto &[name, p] : printer_drivers)
    {
        bool cashdrawer;
        if (fromSettings("printer_" + name + "_cdrawer", cashdrawer))
            p->setCashDrawerEnabled(cashdrawer);
        bool cutter;
        if (fromSettings("printer_" + name + "_cutter", cutter))
            p->setCutterEnabled(cutter);
        int feedlines;
        if (fromSettings("printer_" + name + "_feedlines", feedlines))
            p->setFeedLines(feedlines);
        int pixelwidth;
        if (fromSettings("printer_" + name + "_pixelwidth", pixelwidth))
            p->setPixelWidth(pixelwidth);
        int standard;
        if (fromSettings("printer_" + name + "_standard", standard))
            p->setPrintStandard(standard == 0 ? true : false);
        for (auto &[fname, f] : p->getFields())
        {
            if (f.get_type() == STRING)
            {
                std::string field;
                if (fromSettings("printer_" + name + "_field_" + fname, field))
                    p->setString(fname, field);
            }
            else if (f.get_type() == NUMBER)
            {
                int field;
                if (fromSettings("printer_" + name + "_field_" + fname, field))
                    p->setInt(fname, field);
            }
            else if(f.get_type() == COMBO_LIST)
            {
                std::string field;
                if (fromSettings("printer_" + name + "_field_" + fname, field))
                    p->setCombo(fname, field);
            }
        }
    }
    std::string printer_driver_name;
    if (str_from_settings("current_printer_driver", printer_driver_name))
    {
        setCurrentPrinter(printer_driver_name);
    }
}


