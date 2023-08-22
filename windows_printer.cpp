#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define NOMINMAX



#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "windowsprinter.h"

#include <windows.h>
#include <winspool.h>


DWORD GetVersion(HANDLE handle)
{
    DWORD needed;

    ::GetPrinterDriver(handle, NULL, 2, NULL, 0, &needed);
    if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        return -1;
    }

    std::vector<char> buffer(needed);
    if (::GetPrinterDriver(handle, NULL, 2, (LPBYTE) &buffer[0], needed, &needed) == 0)
    {
        return -1;
    }

    return ((DRIVER_INFO_2A*) &buffer[0])->cVersion;
}


bool IsV4Driver(CHAR *printerName)
{
    HANDLE handle;
    PRINTER_DEFAULTSA defaults;
    char datatype[] = "RAW";
    defaults.DesiredAccess = PRINTER_ACCESS_USE;
    defaults.pDatatype = datatype;
    defaults.pDevMode = NULL;

    if (::OpenPrinterA(printerName, &handle, &defaults) == 0)
    {
        return false;
    }

    DWORD version = GetVersion(handle);

    ::ClosePrinter(handle);

    return version == 4;
}




bool isPrinterOffline(CHAR *printerName){
    HANDLE handle;
    PRINTER_DEFAULTSA defaults;
    char datatype_raw[] = "RAW";
    char datatype_xps_pass[] = "XPS_PASS";
    defaults.DesiredAccess = PRINTER_ACCESS_USE;
    defaults.pDatatype = IsV4Driver(printerName) ? datatype_xps_pass : datatype_raw;
    defaults.pDevMode = NULL;

    if (::OpenPrinterA(printerName, &handle, &defaults) == 0)
    {
        return true;
    }
    unsigned char buffer[2000];
    unsigned long needed;
    if (!GetPrinterA(handle, 2, buffer, 2000, &needed)){
        std::cout << "Buffer too small, adjust\n";
        return true;
    }

    PRINTER_INFO_2A *printer_info = (PRINTER_INFO_2A*) buffer;

    if (printer_info->Attributes & (PRINTER_STATUS_OFFLINE | PRINTER_ATTRIBUTE_WORK_OFFLINE | PRINTER_STATUS_NOT_AVAILABLE  ))
    {
            return true;
    }

//    std::cout << "Printer " << printer_info->pPrinterName << " status is: " << printer_info->Status << std::endl;
    return false;
}

device_status PrinterWindowsSpooler::updateAndGetStatus()
{
    return isPrinterOffline(fields.at("name")->get_string().data()) ? DISCONNECTED : CONNECTED;
}

std::vector<std::string> PrinterWindowsSpooler::enumeratePrinters()
{
    std::vector<std::string> printers;
    unsigned long pcbneeded = 0;
    unsigned long pcbreturned = 0;
    unsigned char buffer[10000];
    EnumPrintersA(
      PRINTER_ENUM_LOCAL,
      NULL,
      5,
      buffer,
      10000,
      &pcbneeded,
      &pcbreturned
    );

    PRINTER_INFO_5A *pinfo = (PRINTER_INFO_5A *) buffer;
    for (int i = 0; i < pcbreturned ; i++)
    {
        printers.push_back(pinfo[i].pPrinterName);
//        std::cout << pinfo[i].pPrinterName << std::endl;
    }
    return printers;
}


//std::string PrinterWindowsRawSpooler::getName() const {return fields.at("Name").get_combo_selected();};


bool PrinterWindowsSpooler::send_raw(const std::string &buffer2)
{
  BOOL bStatus = FALSE;
  HANDLE hPrinter = NULL;
  DOC_INFO_1A DocInfo;
  DWORD dwPrtJob = 0L;
  DWORD dwBytesWritten = 0L;

  unsigned char *lpData = new unsigned char[buffer2.size()];
  memcpy_s(lpData, buffer2.size(), buffer2.data(), buffer2.size());

  auto printer_name = fields.at("name")->get_string();

//  char *lpData = new char[buffer2.size()];
//  strcpy_s(lpData, buffer2.size() + 1, buffer2.c_str());


//  LPSTR lpData = new char[buffer.size()];
//  strcpy_s(lpData, buffer.size()+1, buffer.c_str());
  DWORD dwCount = buffer2.size();

  // Open a handle to the printer.
  bStatus = OpenPrinterA(printer_name.data(), &hPrinter, NULL);

  if (bStatus) {
    // Fill in the structure with info about this "document."
    CHAR document_name[] = "My Document";
    DocInfo.pDocName =  document_name;
    DocInfo.pOutputFile = NULL;

    // Enter the datatype of this buffer.
    //  Use "XPS_PASS" when the data buffer should bypass the
    //    print filter pipeline of the XPSDrv printer driver.
    //    This datatype would be used to send the buffer directly
    //    to the printer, such as when sending print head alignment
    //    commands. Normally, a data buffer would be sent as the
    //    "RAW" datatype.
    //
    CHAR RAW[] = "RAW";
    CHAR XPS_PASS[] = "XPS_PASS";
    DocInfo.pDatatype = IsV4Driver(printer_name.data()) ? XPS_PASS : RAW;

    dwPrtJob = StartDocPrinterA(
        hPrinter,
        1,
        (LPBYTE)&DocInfo);

    std::cout << "LPDATA: " << lpData << std::endl;

    if (dwPrtJob > 0) {
      std::cout << "Sending Data....\n";
      // Send the data to the printer.
      bStatus = WritePrinter(
            hPrinter,
            lpData,
            dwCount,
            &dwBytesWritten);
    std::cout << "Bytes written: " << dwBytesWritten << std::endl;
    }
    EndPagePrinter(hPrinter);
    EndDocPrinter(hPrinter);

    // Close the printer handle.
    bStatus = ClosePrinter(hPrinter);

  }

  if (!bStatus || (dwCount != dwBytesWritten)) {
    bStatus = FALSE;
  } else {
    bStatus = TRUE;
  }
  delete [] lpData;
  return bStatus;
}


PrinterWindowsSpooler::~PrinterWindowsSpooler()
{

};
#endif
