#include <cstdlib>

#include "HttpServerClass.h"

int main(int argc, char **argv){
  if ( argc < 2 ) return -1;
  int port = atoi(argv[1]);
  HttpServer server(port);

  try {
    server.run();
  } catch(Exception &e) {
    e.Report();
  }

  return 0;
}
