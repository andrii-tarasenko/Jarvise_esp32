#ifndef LOCAL_WEB_SERVER_H
#define LOCAL_WEB_SERVER_H

#include "Env.h"
#include <WebServer.h>

class RelayController;
class ClimateSensor;

class LocalWebServer {
public:
  LocalWebServer(int port, RelayController *relayCtrl, ClimateSensor *climateSensor);
  void begin();
  void handleClient();

private:
  WebServer server;
  RelayController* relayController;
  ClimateSensor* climateSensor;

  void handlePostRelay();
  void handlePostClimate();
  void handleNotFound();
};

#endif
