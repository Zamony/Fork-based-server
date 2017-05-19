#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "SockLib.h"

using namespace std;

void Exception::Report(){
  cerr << "<Error (" << GetMessage() << " )> : ";
  perror(NULL);
  cerr << endl;
}

std::string SocketException::GetMessage(){
  return string(SocketException::m_Message[m_ErrCode]);
}
const char * SocketException::m_Message[] = {
  "Socket success",
  "Socket creation error",
  "Socket connection error",
  "Socket is illegal",
  "Socket uses incorrect hostname",
  "Socket data send error",
  "Socket data recv error",
  "Socket bind error",
  "Socket listen error",
  "Socket accept error",
};

///////////////////////////////////////////////////////////////

TcpServer::TcpServer(int pn, int bs) {
  port = pn;
  backlog_size = bs;
}

void TcpServer::readSock(int sock){
  const int len = 4096;
  char buf[len] = {0};
  read(sock, buf, len);
  p_req->setData(buf);
  if (strlen(buf) == 0) exit(0);
  string t = buf;
  cout << t << endl;
}

void TcpServer::writeSock(int sock){
  string s_resp = p_resp->getData();
  const char *s = s_resp.c_str();
  write(sock, s, s_resp.length());
}
void TcpServer::handleRequest(){
  cout << "ERROR IN HANDLE\n";
}

void TcpServer::prepareSocket(){
  if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    throw SocketException(SocketException::ESE_SOCKCREATE);

  int enable = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    throw SocketException(SocketException::ESE_SOCKCREATE);

  struct sockaddr_in addr_struct;
  memset(&addr_struct, 0, sizeof(addr_struct));
  addr_struct.sin_port = htons(port);
  addr_struct.sin_family = AF_INET;
  addr_struct.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");

  if ( bind(sd, (struct sockaddr *) &addr_struct, sizeof(addr_struct)) != 0 )
    throw SocketException(SocketException::ESE_SOCKBIND);

  if ( listen(sd, backlog_size) < 0 )
    throw SocketException(SocketException::ESE_SOCKLISTEN);
}


int TcpServer::run(){
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;
  char str_ip[INET_ADDRSTRLEN];

  prepareSocket();
  int sock;
  for(;;){
    if ( (sock = accept(sd, (struct sockaddr *)&their_addr, &addr_size)) < 0  )
      throw SocketException(SocketException::ESE_SOCKACCEPT);

    // Fill remote info
    sockaddr_in* p_addr = (sockaddr_in *) &their_addr;
    inet_ntop( AF_INET, &(p_addr->sin_addr), str_ip, INET_ADDRSTRLEN );
    p_req->client_ip = str_ip;
    p_req->client_port = ntohs(p_addr->sin_port);


    cout << "Accepted\n";
    if ( fork() ){
      close(sock);
    } else {
      cout << "Started\n";
      close(sd);
      readSock(sock);
      handleRequest();
      cout << "Handled\n";
      writeSock(sock);
      shutdown(sock, 2);
      close(sock);
      cout << "Finished\n\n\n\n\n";
      exit(0);
      cout << "ERROR FINISHING\n";
    }
  }

  return 0;
}
