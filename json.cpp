#include <string>
#include "rapidjson/document.h"
#include "base64.hpp"
#include "messagesystem.h"

using namespace rapidjson;

namespace json
{
    const std::string getJsonStatusString(const char* request)
    {
        // 1. Parse a JSON string into DOM.
//        std:: cout << "Got string: " << request << std::endl;
        Document d;
        ParseResult ok = d.Parse(request);
        if (!ok) {
            fprintf(stderr, "JSON parse error");
            return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
        }
        GlobalState::updatePrinterStatus();

        // 3. Stringify the DOM
        Value::ConstMemberIterator itr = d.FindMember("id");
        if (itr != d.MemberEnd())
        {
            std::string ID = std::to_string(itr->value.IsInt() ? itr->value.GetInt() : -1);
            return "{\"jsonrpc\": \"2.0\", \"id\": " + ID + ", \"result\": {\"printer\": {\"status\": \"" + to_string(GlobalState::getPrinterStatus()) + "\", \"messages\": \"\"}, \"scanner\": {\"status\": \"disconnected\", \"messages\": \"\"}}}";
        }
        else return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
    }

    const std::string getResultTrueString(const char* request)
    {
        // 1. Parse a JSON string into DOM.
//        std:: cout << "Got string: " << request << std::endl;
        Document d;
        ParseResult ok = d.Parse(request);
        if (!ok) {
            fprintf(stderr, "JSON parse error");
            return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
        }

        // 3. Stringify the DOM
        Value::ConstMemberIterator itr = d.FindMember("id");
        if (itr != d.MemberEnd())
        {
            std::string ID = std::to_string(itr->value.IsInt() ? itr->value.GetInt() : -1);
 //           std::cout << "ID: " << ID << std::endl;

            return "{\"jsonrpc\": \"2.0\", \"id\": " + ID + ", \"result\": true}";
        }
        else return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
    }

    const std::string PrinterDefaultAction(const char* request)
    {
        std::string ID;
        Document d;
        ParseResult ok = d.Parse(request);
        if (!ok) {
            fprintf(stderr, "JSON parse error");
            return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";
        }
        Value::ConstMemberIterator itr = d.FindMember("id");
        if (itr != d.MemberEnd())
        {
            ID = std::to_string(itr->value.IsInt() ? itr->value.GetInt() : -1);
        }
        else return "{\"jsonrpc\": \"2.0\", \"id\": 0, \"result\": false}";

        itr = d.FindMember("params");
        if (itr != d.MemberEnd())
        {
            const Value &params = itr->value;
            if (params.IsObject())
            {
                itr = params.FindMember("data");
                if (itr != params.MemberEnd())
                {
                    const Value &data = itr->value;
                    if (data.IsObject())
                    {
                        itr = data.FindMember("action");
                        if (itr != data.MemberEnd())
                        {
                            if (itr->value.IsString())
                            {
                                if (itr->value == "print_receipt")
                                {
                                    itr = data.FindMember("receipt");
                                    if (itr != data.MemberEnd())
                                    {
                                        const Value &receipt = itr->value;
                                        if (receipt.IsString())
                                        {
                                            std::string receipt_string = std::string(receipt.GetString());
                                            //DEBUG
                                            /*

                                            std::fstream file;
                                            try {
                                                file.exceptions ( std::ofstream::badbit | std::ofstream::failbit );
                                                file.open("receipt.jpg", std::ios::out | std::ios::binary);
                                                std::string jpeg_binary_data = base64::Decode(receipt_string);
                                                file.write(jpeg_binary_data.c_str(), jpeg_binary_data.size());
//                                                file << base64::Decode(receipt_string);
                                            }
                                            catch (const std::ofstream::failure& e) {
                                                std::cout << "Failure to write binary file test\n";
                                                if (file.is_open()) file.close();
                                            }
                                            if (file.is_open()) file.close();
                                            */
                                            //DEBUG
                                            GlobalState::enqueuePrintJob(base64::Decode(receipt_string), JPEG);


                                            return "{\"jsonrpc\": \"2.0\", \"id\": " + ID + ", \"result\": true}";
                                        }
                                    }
                                }
                                else if (itr->value == "cashbox")
                                {
//                                    std::cout << "Got CashBox Open Request\n";
//                                    std::string temp = "dummy";
                                    GlobalState::enqueuePrintJob("dummy", CASHDRAWER);
                                    return "{\"jsonrpc\": \"2.0\", \"id\": " + ID + ", \"result\": true}";
                                }
                            }
                        }
                    }
                }
            }
        }
        return "{\"jsonrpc\": \"2.0\", \"id\": " + ID + ", \"result\": false}";
    }
}
