#ifndef SOCKLIB_H
#define SOCKLIB_H


class Exception {
protected:
    int m_ErrCode;
public:
    Exception(int errcode) : m_ErrCode(errcode) {}
    void Report();
    virtual std::string GetMessage() = 0;
};

class SocketException : public Exception {
    static const char * m_Message[10];
public:
    enum SocketExceptionCode {
         ESE_SUCCESS,
         ESE_SOCKCREATE,
         ESE_SOCKCONN,
         ESE_SOCKILLEGAL,
         ESE_SOCKHOSTNAME,
         ESE_SOCKSEND,
         ESE_SOCKRECV,
         ESE_SOCKBIND,
         ESE_SOCKLISTEN,
         ESE_SOCKACCEPT,
    };
    SocketException(SocketExceptionCode errcode) : Exception(errcode) {}
    std::string  GetMessage();
};

class TcpRequest {
  std::string data;
public:
  std::string client_ip;
  int client_port;
  virtual void setData(const char s[]){}
};
class TcpResponse {
  std::string data;
public:
  virtual std::string getData(){
    return "";
  }
};

class TcpServer {
  protected:
    int port;
    int backlog_size;
    int sd;
    void prepareSocket();
    void readSock(int);
    void writeSock(int);
    virtual void handleRequest();
  public:
    TcpResponse* p_resp;
    TcpRequest* p_req;
    TcpServer(int, int bs = 5);
    int run();
};

#endif
