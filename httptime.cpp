#include "httptime.hpp"

namespace httptime
{

const char *dayname[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char *monthname[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *textnumbers[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
   std::string now()
   {
       time_t now = time(0);
       // convert now to tm struct for UTC
       tm *gmtm = gmtime(&now);
       std::stringstream output;
       output << dayname[gmtm->tm_wday] << ", " << gmtm->tm_mday\
              << " " << monthname[gmtm->tm_mon] << " " \
              << std::to_string(gmtm->tm_year + 1900) << " " \
              << textnumbers[gmtm->tm_hour/10] << textnumbers[gmtm->tm_hour%10] << ":" \
              << textnumbers[gmtm->tm_min/10] << textnumbers[gmtm->tm_min%10] << ":" \
              << textnumbers[gmtm->tm_sec/10] << textnumbers[gmtm->tm_sec%10] << " GMT";
       return output.str();
   }
}
