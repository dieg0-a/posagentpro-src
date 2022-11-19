#include "App.h"
#include <chrono>
#include "json.hpp"
#include "httptime.hpp"
#include <thread>
#include <mainwindow.h>
#include <QApplication>
#include "messagesystem.h"


int networkThread(int http_port)
{
//  PrinterDummy *p = new PrinterDummy();
//  PrinterWindowsSpooler *p = new PrinterWindowsSpooler("POS-80-Series");
//  hardware::InstallPrinter(p);
//  auto printers = PrinterWindowsSpooler::enumeratePrinters();
  /* Overly simple hello world app */
  uWS::App({
      .key_file_name = "misc/key.pem",
      .cert_file_name = "misc/cert.pem",
      .passphrase = "1234",
  })
      .get("/hw_proxy/hello", [] (auto *res, auto*) {
        res->writeHeader("Content-Type", "text/html; charset=utf-8");
        res->writeHeader("Access-Control-Allow-Origin", "*");
        res->writeHeader("Access-Control-Allow-Methods", "GET, POST");
        res->writeHeader("Date", httptime::now());
        res->writeHeader("Connection", "close");
//        res->writeHeader("Connection", "close");
        res->end("ping");
      })
      .post("/hw_proxy/handshake", [] (auto *res, auto*) {
      //You also need to set onAborted if receiving data
      res->onData([res, bodyBuffer = (std::string *)nullptr](std::string_view chunk, bool isLast) mutable {
          if (isLast) {
              if (bodyBuffer) {
                  // Send back the (chunked) body we got, as response
                  bodyBuffer->append(chunk);
                  res->writeHeader("Content-Type", "application/json");
                  res->writeHeader("Access-Control-Allow-Origin", "*");
//                    res->writeHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization, X-Debug-Mode");
                  res->writeHeader("Access-Control-Allow-Methods", "POST");
                  res->writeHeader("Date", httptime::now());
                  res->writeHeader("Connection", "close");
//                    res->writeHeader("Connection", "close");
                  res->end(json::getResultTrueString(bodyBuffer->c_str()));
//					res->end(*bodyBuffer);
                  delete bodyBuffer;
              } else {
                  // Send back the body we got, as response (fast path)
                  res->writeHeader("Content-Type", "application/json");
                  res->writeHeader("Access-Control-Allow-Origin", "*");
//                    res->writeHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization, X-Debug-Mode");
                  res->writeHeader("Access-Control-Allow-Methods", "POST");
                  res->writeHeader("Date", httptime::now());
                  res->writeHeader("Connection", "close");
//                    res->writeHeader("Connection", "close");
                  bodyBuffer = new std::string;
                  bodyBuffer->append(chunk);
                  bodyBuffer->append("\0");
                  res->end(json::getResultTrueString(bodyBuffer->c_str()));
                  delete bodyBuffer;
              }
          } else {
              // Slow path
              if (!bodyBuffer) {
                  bodyBuffer = new std::string;
              }
              // If we got the body in a chunk, buffer it up until whole
              bodyBuffer->append(chunk);
          }
      });

      // If you have pending, asynch work, you should abort such work in this callback
      res->onAborted([]() {
          // Again, just printing is not enough, you need to abort any pending work here
          // so that nothing will call res->end, since the request was aborted and deleted
          printf("Stream was aborted!\n");
      });
      })
      .options("/hw_proxy/*", [] (auto *res, auto*) {
          res->writeHeader("Content-Type", "text/html; charset=utf-8");
          res->writeHeader("Access-Control-Allow-Origin", "*");
          res->writeHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization, X-Debug-Mode");
          res->writeHeader("Access-Control-Max-Age", "86400");
          res->writeHeader("Access-Control-Allow-Methods", "POST");
          res->writeHeader("Date", httptime::now());
          res->writeHeader("Connection", "close");
//          res->writeHeader("Connection", "close");
          res->end();
      })
      .post("/hw_proxy/status_json", [] (auto *res, auto*) {
        //You also need to set onAborted if receiving data
        res->onData([res, bodyBuffer = (std::string *)nullptr](std::string_view chunk, bool isLast) mutable {
            if (isLast) {
                if (bodyBuffer) {
                    // Send back the (chunked) body we got, as response
                    bodyBuffer->append(chunk);
                    res->writeHeader("Content-Type", "application/json");
                    res->writeHeader("Access-Control-Allow-Origin", "*");
//                    res->writeHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization, X-Debug-Mode");
                    res->writeHeader("Access-Control-Allow-Methods", "POST");
                    res->writeHeader("Date", httptime::now());
                    res->writeHeader("Connection", "close");
//                    res->writeHeader("Connection", "close");
                    res->end(json::getJsonStatusString(bodyBuffer->c_str()));
//					res->end(*bodyBuffer);
                    delete bodyBuffer;
                } else {
                    // Send back the body we got, as response (fast path)
                    res->writeHeader("Content-Type", "application/json");
                    res->writeHeader("Access-Control-Allow-Origin", "*");
//                    res->writeHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization, X-Debug-Mode");
                    res->writeHeader("Access-Control-Allow-Methods", "POST");
                    res->writeHeader("Date", httptime::now());
                    res->writeHeader("Connection", "close");
//                    res->writeHeader("Connection", "close");
                    bodyBuffer = new std::string;
                    bodyBuffer->append(chunk);
                    bodyBuffer->append("\0");
                    res->end(json::getJsonStatusString(bodyBuffer->c_str()));
                    delete bodyBuffer;
                }
            } else {
                // Slow path
                if (!bodyBuffer) {
                    bodyBuffer = new std::string;
                }
                // If we got the body in a chunk, buffer it up until whole
                bodyBuffer->append(chunk);
            }
        });

        res->onAborted([]() {

            printf("Stream was aborted!\n");
        });
      })
      .post("/hw_proxy/default_printer_action", [](auto *res, auto *req){
      //You also need to set onAborted if receiving data
      res->onData([res, bodyBuffer = (std::string *)nullptr](std::string_view chunk, bool isLast) mutable {
          if (isLast) {
              if (bodyBuffer) {
                  // Send back the (chunked) body we got, as response
                  bodyBuffer->append(chunk);
                  res->writeHeader("Content-Type", "application/json");
                  res->writeHeader("Access-Control-Allow-Origin", "*");
//                    res->writeHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization, X-Debug-Mode");
                  res->writeHeader("Access-Control-Allow-Methods", "POST");
                  res->writeHeader("Date", httptime::now());
                  res->writeHeader("Connection", "close");
//                    res->writeHeader("Connection", "close");
                  bodyBuffer->append("\0");
                  res->end(json::PrinterDefaultAction(bodyBuffer->c_str()));
                  delete bodyBuffer;
              } else {
                  // Send back the body we got, as response (fast path)
                  res->writeHeader("Content-Type", "application/json");
                  res->writeHeader("Access-Control-Allow-Origin", "*");
//                    res->writeHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization, X-Debug-Mode");
                  res->writeHeader("Access-Control-Allow-Methods", "POST");
                  res->writeHeader("Date", httptime::now());
                  res->writeHeader("Connection", "close");

                  bodyBuffer = new std::string;
                  bodyBuffer->append(chunk);
                  bodyBuffer->append("\0");
                  res->end(json::PrinterDefaultAction(bodyBuffer->c_str()));
                  delete bodyBuffer;
              }
          } else {
              // Slow path
              if (!bodyBuffer) {
                  bodyBuffer = new std::string;
              }
              // If we got the body in a chunk, buffer it up until whole
              bodyBuffer->append(chunk);
          }
      });


      res->onAborted([]() {

          printf("Stream was aborted!\n");
      });
      })


      .listen(http_port, [](auto *listen_socket) {
        if (listen_socket) {
//          std::cout << "Listening on port " << 9069 << std::endl;
          GlobalState::setNetworkListenSocket(listen_socket);
        }
      })
      .run();
//  delete p;

  std::cout << "Network thread fell through!\n";
  return 0;
}

int main(int argc, char *argv[]) {
//    std::thread network_thread(networkThread);
    QApplication a(argc, argv);
    MainWindow w;
//    w.show();
    return a.exec();
}


