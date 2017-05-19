#ifndef CGICOMPONENTCLASS_H
#define CGICOMPONENTCLASS_H

#include <string>
#include <map>

class HttpRequest;

class CGIComponent {
  std::map<std::string, std::string> handlers;
  std::string srv_name;
  int srv_port;
  std::string srv_ip;
  static const int VARIABLES_COUNT = 15;
  char *env[VARIABLES_COUNT + 1] = {0};

public:
  const char *dir = "./cgi-bin";
  const char *temp_dir = "./temp";
  const char *installation_dir = "/home/zamony/C++/ExServer";

  CGIComponent(std::string sname, int sport, std::string sip);
  CGIComponent(){}
  ~CGIComponent();
  void setEnvVariables(const HttpRequest & req, std::string program);
  int launchInterpreter(const HttpRequest & req, std::string program);
  int loadResource(const HttpRequest & req);
};

#endif
