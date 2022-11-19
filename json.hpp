#include <string>
#include <iostream>
#include "hardware.hpp"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace json
{
    const std::string getJsonStatusString(const char* request);
    const std::string getResultTrueString(const char* request);
    const std::string PrinterDefaultAction(const char* request);
}
