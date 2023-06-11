#ifndef MESSAGESYSTEM_H
#define MESSAGESYSTEM_H
#include <string>
#include <thread>
#include <queue>

#include <mutex>
#include "printer.hpp"
#include "windowsprinter.h"

#include "App.h"
#include "jpeg.hpp"



int networkThread(int);

enum json_rpc_type {PROXY_HELLO, PROXY_HANDSHAKE, PROXY_STATUS, PRINTER_DEFAULT_ACTION};
enum print_job_type {JPEG, CASHDRAWER};


class GlobalState
{
private:
    static device_status printer_status;
    static std::mutex state_mutex;
    static int current_printer_index;
    static Printer *selected_printer;
    static std::queue<std::pair<std::string, print_job_type>> raw_job_queue;
    static std::map<std::string, Printer*> printer_drivers;
    static std::thread *network_thread;
    static us_listen_socket_t *http_server_socket;
    static std::atomic<uWS::Loop *> network_loop;
    static bool network_thread_running;
    static int http_proxy_port;

    static jpeg *last_jpeg_receipt;

    ////PROGRAM SETTINGS HOOKS. TO BE SET DEPENDING ON THE FRAMERWORK/PLATFORM/GUI SPECIFIC CODE (IE: QT)


    static std::function<bool(const std::string &key, std::string &val)> str_from_settings;
    static std::function<bool(const std::string &key, int &val)> int_from_settings;
    static std::function<bool(const std::string &key, bool &val)> bool_from_settings;

    static std::function<bool(const std::string &key, const std::string &val)> str_to_settings;
    static std::function<bool(const std::string &key, int val)> int_to_settings;
    static std::function<bool(const std::string &key, bool val)> bool_to_settings;


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    static PrinterWindowsSpooler winprint;
#endif

#ifdef __linux__
    static PrinterLinuxUSBRAW linux_usb_print;
    static PrinterThermalLinuxTCPIP linux_ip_print;
#endif


public:

    static bool autosave;
    static std::atomic<bool> demo_mode;
    static int demo_print_jobs;

    static jpeg *getLastReceipt() { return last_jpeg_receipt;};

    static int getHttpPort() {return http_proxy_port;};


    static bool getCashDrawerEnabled() {
        if (selected_printer != nullptr)
        {
            return selected_printer->getCashDrawerEnabled();
        }
        else return 0;
    };

    static void printerSetName(const std::string &n) {
        state_mutex.lock();
        if (selected_printer != nullptr) selected_printer->setName(n);
        state_mutex.unlock();
    };

    static void printerSetInt(std::string name, int i) {
        if (selected_printer != nullptr)
        {
            selected_printer->setInt(name,i);
            if (autosave) int_to_settings("printer_" + getCurrentPrinterName() + "_field_" + name, i);
        }
    };

    static void printerSetString(std::string name, std::string s) {
        if (selected_printer != nullptr)
        {
            selected_printer->setString(name,s);
            if (autosave)
                str_to_settings("printer_" + getCurrentPrinterName() + "_field_" + name, s);
        }
    };

    static void printerSetIndex(std::string name, int index) {
        if (selected_printer != nullptr)
        {
            selected_printer->setIndex(name,index);
            if (autosave)
                str_to_settings("printer_" + getCurrentPrinterName() + "_field_" + name, selected_printer->getString(name));
        }
    };


    static std::map<std::string, input_field*> &printerGetFields() {
        return selected_printer->getFields();
    };
    static int printerGetInt(std::string name) {
        return selected_printer->getInt(name);
    };
    static std::string printerGetString(std::string name) {
        return selected_printer->getString(name);
    };
    static std::vector<std::string> &printerGetStrList(std::string name) {
        return selected_printer->getListOfStrings(name);
    };

    static void setHttpPort(int port)
    {
        http_proxy_port = port;
        if (autosave)
        {
            int_to_settings("http_proxy_port", port);
        }
    }

    static bool setSettingsHooks(std::function<bool(const std::string &key, std::string &val)> _str_from_settings,
                                 std::function<bool(const std::string &key, int &val)> _int_from_settings,
                                 std::function<bool(const std::string &key, bool &val)> _bool_from_settings,
                                 std::function<bool(const std::string &key, const std::string &val)> _str_to_settings,
                                 std::function<bool(const std::string &key, int val)> _int_to_settings,
                                 std::function<bool(const std::string &key, bool val)> _bool_to_settings)
    {
        str_from_settings = _str_from_settings;
        int_from_settings = _int_from_settings;
        bool_from_settings = _bool_from_settings;
        str_to_settings = _str_to_settings;
        int_to_settings = _int_to_settings;
        bool_to_settings = _bool_to_settings;
        return true;
    }

    static bool fromSettings(const std::string &key, std::string &value)
    {
        return str_from_settings(key, value);
    }
    static bool fromSettings(const std::string &key, int &value)
    {
        return int_from_settings(key,value);
    }
    static bool fromSettings(const std::string &key, bool &value)
    {
        return bool_from_settings(key,value);
    }


    static bool toSettings(const std::string &key, const std::string &value)
    {
        return str_to_settings(key,value);
    }

    static bool toSettings(const std::string &key, int value)
    {
        return int_to_settings(key, value);
    }
    static bool toSettings(const std::string &key, bool value)
    {
        return bool_to_settings(key,value);
    }

    static void loadSettings();

    static void saveSettings();

    static void startNetworkThread()
    {
        if (!network_thread_running)
        network_thread = new std::thread(networkThread, http_proxy_port);
        network_thread_running = true;
    }

    static void stopNetworkThread()
    {
        if (network_thread_running)
        {
            state_mutex.lock();
            if (http_server_socket != nullptr)
            {
                uWS::Loop *l = network_loop;
                if (l != nullptr) l->defer(stopNetworkLoop);
            }
            else
            {
                state_mutex.unlock();
                return;
            }
            state_mutex.unlock();
            network_thread->join();
            network_thread_running = false;
            delete network_thread;
        }
    }

    static void stopNetworkLoop() {  //Defer call to network thread!!
         us_listen_socket_close(0, http_server_socket);
    }
    static void setNetworkListenSocket(us_listen_socket_t *socket)
    {
        state_mutex.lock();
        http_server_socket = socket;
        state_mutex.unlock();
    }

    static void setNetworkLoop(uWS::Loop *l){
        network_loop = l;
    }

    static std::vector<std::string> printerDrivers()
    {
        std::vector<std::string> drivers;
        for (auto &[i, j] : printer_drivers)
        {
            drivers.push_back(i);
        }
        return drivers;
    }

    static std::string driverAt(int index)
    {
        auto i =  printer_drivers.begin();
        if (index < printer_drivers.size())
        {
            for (int j = 0; j < index ; j++) i++;
            return i->first;
        }
        else return "";
    }

    static void registerPrinter(std::string name, Printer *p){
        printer_drivers.emplace(name, p);
        if (printer_drivers.size() == 1)
        {
            state_mutex.lock();
            selected_printer = p;
            state_mutex.unlock();
        }
    };

    static void setCurrentPrinter(std::string name)
    {
        int i = 0;
        auto p = printer_drivers.begin();
        while (p != printer_drivers.end())
        {
            if (p->first == name)
            {
                current_printer_index = i;
                state_mutex.lock();
                selected_printer = p->second;
                state_mutex.unlock();
                return;
            }
            i++;
            p++;
        }
        if (autosave)
            str_to_settings("current_printer_driver",  getCurrentPrinterName());
    }

    static void setCurrentPrinter(int index)
    {
        if (index < printer_drivers.size())
        {
            auto p = printer_drivers.begin();
            for (int i = 0; i < index ; i++) p++;
            state_mutex.lock();
            selected_printer = p->second;
            state_mutex.unlock();
            current_printer_index = index;
        }
        if (autosave)
            str_to_settings("current_printer_driver", getCurrentPrinterName());
    }

    static Printer *getCurrentPrinter(){return selected_printer;};

    static std::string getCurrentPrinterName();

    static device_status getPrinterStatus()
    {
        state_mutex.lock();
        auto temp = printer_status;
        state_mutex.unlock();
        return temp;
    }

    static void updatePrinterStatus()
    {
        if (selected_printer != nullptr)
        {
            device_status temp = (selected_printer == nullptr) ? DISCONNECTED :
                                                                selected_printer->updateAndGetStatus();
            state_mutex.lock();
            printer_status = temp;
            state_mutex.unlock();
        }
    }
    static bool enqueuePrintJob(std::string &&rawdata, print_job_type type);
//    static bool enqueuePrintJob(std::string &rawdata, print_job_type type);

    static bool enqueueJPEGPrintJob(std::string &rawdata);

    static bool setLastReceiptImage(std::string &jpeg_data);

//    static void setCurrentPrinter(Printer *p);

    static bool processQueue();

    GlobalState();
};

#endif // MESSAGESYSTEM_H
