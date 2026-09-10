#include "base64.hpp"
#include "messagesystem.h"
#include <string>

#include "json/single_include/nlohmann/json.hpp"

using nlohmann_json = nlohmann::json;

namespace json {
const std::string getJsonStatusString(const char *request) {
  // 1. Parse a JSON string into DOM.
  //        std:: cout << "Got string: " << request << std::endl;

  try {
    nlohmann_json req = nlohmann_json::parse(request);
    nlohmann_json id = req.at("id");
    if (id.is_number()) {
        auto id_str = std::to_string(id.get<int>());
//      nlohmann_json params = req.at("params");

      return "{\"jsonrpc\": \"2.0\", \"id\": " +
             id_str +
             ", \"result\": {\"printer\": {\"status\": \"" +
             to_string(GlobalState::getPrinterStatus()) +
             "\", \"messages\": \"\"}, \"scanner\": {\"status\": "
             "\"disconnected\", \"messages\": \"\"}}}";
    }
    return "{\"jsonrpc\": \"2.0\", \"id\":0, \"result\": false}";
  }

  catch (const nlohmann_json::exception &e) {
    // output exception information
    std::cout << "message: " << e.what() << '\n'
              << "exception id: " << e.id << std::endl;
    fprintf(stderr, "JSON parse error");
    return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
  }
}

const std::string getResultTrueString(const char *request) {
  // 1. Parse a JSON string into DOM.
  //        std:: cout << "Got string: " << request << std::endl;

  try {
    nlohmann_json req = nlohmann_json::parse(request);
    nlohmann_json id = req.at("id");
    auto id_str = std::to_string(id.get<int>());
    if (id.is_number()) {
      return "{\"jsonrpc\": \"2.0\", \"id\": " +
             id_str + ", \"result\": true}";
    }
    return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": true}";
  } catch (const nlohmann_json::exception &e) {
    // output exception information
    std::cout << "message: " << e.what() << '\n'
              << "exception id: " << e.id << std::endl;
    fprintf(stderr, "JSON parse error");
    return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
  }

  return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
}

const std::string PrinterDefaultAction(const char *request) {
  // 1. Parse a JSON string into DOM.
  //        std:: cout << "Got string: " << request << std::endl;

  try {
    nlohmann_json req = nlohmann_json::parse(request);
    nlohmann_json id = req.at("id");
    if (id.is_number()) {
      nlohmann_json params = req.at("params");
      nlohmann_json data = params.at("data");
      nlohmann_json action = data.at("action");
      auto action_string = action.template get<std::string>();
      if (action_string == "print_receipt") {
//        nlohmann_json receipt = data.at("receipt");
        std::string receipt_data = data.at("receipt");
        GlobalState::enqueuePrintJob(base64::Decode(receipt_data), JPEG);
      } else if (action_string == "print_receipt_JSON") {
        nlohmann_json receipt = data.at("receipt");
        auto receipt_data = nlohmann::to_string(receipt);
        GlobalState::enqueuePrintJob(std::move(receipt_data), JSON);
      } else if (action_string == "cashbox") {
        GlobalState::enqueuePrintJob("dummy", CASHDRAWER);
        return "{\"jsonrpc\": \"2.0\", \"id\": " +
               id.template get<std::string>() + ", \"result\": true}";
      }
    }
    return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": true}";
  } catch (const nlohmann_json::exception &e) {
    // output exception information
    std::cout << "message: " << e.what() << '\n'
              << "exception id: " << e.id << std::endl;
    fprintf(stderr, "JSON parse error");
    return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
  }

  return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
}

// const std::string GetPrinterList(const char *request) {
//     try {
//         nlohmann_json req = nlohmann_json::parse(request);
//         nlohmann_json id = req.at("id");
//         if (id.is_number()) {
//           return "{\"jsonrpc\": \"2.0\", \"id\": " +
//                  id.template get<std::string>() + ", \"result\": false}";
//         }
//         return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
//       } catch (const nlohmann_json::exception &e) {
//         // output exception information
//         std::cout << "message: " << e.what() << '\n'
//                   << "exception id: " << e.id << std::endl;
//         fprintf(stderr, "JSON parse error");
//         return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
//       }
    
//       return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
// }
} // namespace responseHandle
