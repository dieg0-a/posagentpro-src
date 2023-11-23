#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <vector>

struct label_info {
  std::string pname;
  std::string barcode;
  bool printprice;
  double unitprice;
  std::vector<std::tuple<std::string, double, double>> pricepoints;
};

#endif