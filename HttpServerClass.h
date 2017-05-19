#ifndef HTTPSERVERCLASS_H
#define HTTPSERVERCLASS_H

#include <string>

#include "SockLib.h"
#include "CGIComponentClass.h"

class HttpRequest : public TcpRequest{
  private:
    std::string method;
    std::string path;
    std::string path_main;
    std::string path_query;
    std::string user_agent;
    std::string referer;
    std::string host;
    std::string obtainOptionVal(const char *, const std::string &, const std::string &);
  public:
    std::string data;
    virtual void setData(const char s[]);
    HttpRequest(){}
    std::string getMethod() const ;
    std::string getPath() const ;
    std::string getPathMain() const ;
    std::string getPathQuery() const ;
    std::string getHost() const ;
    std::string getReferer() const ;
    std::string getUserAgent() const ;
    int parse(std::string);
};

class HttpResponse : public TcpResponse {
  public:
    std::string data;
    std::string body = "";
    std::string modified;
    long long int content_length = 0;
    HttpResponse(){}
    virtual std::string getData();
    void setStatus(int, std::string);
    void setContentType(std::string);
    void setContentLength();
    void setLastModified();
    void setAllow();
    void setServer();
    void setDate(std::string);
    void setBody();
    void setDefaultBody(const char *s);
};

class HttpServer : public TcpServer {
  private:
    HttpRequest request;
    HttpResponse response;

    // 600 is a bounder cuz status codes are in range of 1..599
    const char *status_codes[600] = { NULL };
    std::string date;
    const int backlog_size = 5;
    const std::string default_directory = "./web";
    CGIComponent cgi;
  public:
    HttpServer(int port);
    void parseRequest(std::string input);
    int loadResource(std::string);
    bool isInAllowedPaths(std::string path);
    int statResource(std::string s);
    int updateDateInfo();
    void configureResponse(int status_code);
    bool tookResource();
    void handleRequest();
};

#endif
