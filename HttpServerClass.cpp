#include <ctype.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <algorithm>

#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include "SockLib.h"
#include "CGIComponentClass.h"
#include "HttpServerClass.h"

using namespace std;

#include "CGIComponentClass.cpp"


string HttpRequest::obtainOptionVal(const char *ns, const string& s, const string &s2){
  const string needle = ns;
  size_t upos = s.find(needle);
  if (upos == string::npos) return "";

  size_t nlpos = s.find_first_of("\r\n", upos);
  if (nlpos == string::npos) return "";

  int pos = upos + needle.length();
  return s2.substr(pos, nlpos - pos);
}

int HttpRequest::parse(string def_dir = "."){
  string s = data;

  unsigned int nl_pos = s.find_first_of("\r\n", 0);
  if (nl_pos == string::npos) return -1;

  string req_mainline = s.substr(0, nl_pos);
  unsigned int space_pos = req_mainline.find_first_of(" ", 0);
  if (space_pos == string::npos) return -1;

  // Method set
  method = req_mainline.substr(0, space_pos);

  unsigned int nspace_pos = req_mainline.find_first_of(" ", space_pos + 1);
  if (nspace_pos == string::npos) return -1;

  // Path
  string rpath = req_mainline.substr(space_pos + 1, nspace_pos - space_pos - 1);
  path = def_dir + rpath;
  size_t qp_pos = rpath.find('?');
  path_query = qp_pos != string::npos ? rpath.substr(qp_pos + 1) : "";
  path_main = qp_pos != string::npos ? rpath.substr(0, qp_pos) : rpath;

  transform(s.begin(), s.end(), s.begin(), ::tolower);

  user_agent = obtainOptionVal("user-agent: ", s, data);
  host = obtainOptionVal("host: ", s, data);
  referer = obtainOptionVal("referer: ", s, data);

  return 0;
}

std::string HttpRequest::getUserAgent() const {
  return user_agent;
}

std::string HttpRequest::getReferer() const {
  return referer;
}

std::string HttpRequest::getMethod() const {
  return method;
}

std::string HttpRequest::getPath() const {
  return path;
}

std::string HttpRequest::getPathMain() const {
  return path_main;
}

std::string HttpRequest::getPathQuery() const {
  return path_query;
}

std::string HttpRequest::getHost() const {
  return host;
}

void HttpRequest::setData(const char s[]){
  data = s;
}



void HttpResponse::setStatus(int code, string msg){
  this->data = "HTTP/1.1 " + to_string(code) + " " + msg + "\r\n";
}
void HttpResponse::setContentType(string path){
  string s;
  if ( ends_with(path, "html") ) s = "text/html";
  else if ( ends_with(path, "jpeg") || ends_with(path, "jpg") ) s = "image/jpeg";
  else s = "text/plain";
  this->data += "Content-type: " + s + " \r\n";
}
void HttpResponse::setContentLength(){
  this->data += "Content-Length: " + to_string(this->content_length)  + " \r\n";
}
void HttpResponse::setLastModified(){
  this->data += "Last-Modified: " + this->modified + " \r\n";
}
void HttpResponse::setAllow(){
  this->data += "Allow: GET, HEAD\r\n";
}
void HttpResponse::setServer(){
  this->data += "Server: lserv/0.01\r\n";
}
void HttpResponse::setDate(string s){
  this->data += "Date: " + s + "\r\n";
}
void HttpResponse::setBody(){
  this->data += body;
  //cout << body << endl << endl << data << endl << endl;
}

void HttpResponse::setDefaultBody(const char *s){
  if ( body == "" ){
    body = "\r\n<html><body>";
    body += s;
    body += "</body></html>";
    //cout << body << endl;
    content_length = body.length() -2;
  }
}

string HttpResponse::getData(){
  return data;
}


HttpServer::HttpServer(int port): TcpServer(port){
  this->response = HttpResponse();
  this->request = HttpRequest();
  p_resp = &response;
  //response.data = "hello";
  p_req = &request;

  cgi = CGIComponent("localhost", 8888, "127.0.0.1");

  this->status_codes[200] = "OK";
  this->status_codes[400] = "Bad request";
  this->status_codes[403] = "Forbidden";
  this->status_codes[404] = "Not Found";
  this->status_codes[500] = "Internal Server Error";
  this->status_codes[501] = "Not Implemented";
  this->status_codes[503] = "Service Unavailable";

}

int HttpServer::loadResource(string path = ""){
  response.body = "";

  if ( path == "" ){
    response.body = "\r\n";
    path = this->request.getPath();
    if ( !this->isInAllowedPaths(path) ) return 0;
  }

  int fd = open(path.c_str(), O_RDONLY);
  if ( fd < 0 ) return -1;

  char ch;
  int n;
  response.content_length = 0;
  while ( (n = read(fd, &ch, 1)) > 0 ){
      this->response.body += ch;
      this->response.content_length += n;
  }
  this->statResource(request.getPath());
  close(fd);
  return 1;
}

bool HttpServer::isInAllowedPaths(string path){
  return path.find("..") == string::npos;
}

int HttpServer::statResource(string s){
  struct stat sb;
  stat(s.c_str(), &sb);
  struct tm *ptm;
  ptm = gmtime(&sb.st_mtime);
  char buf[300] = {0};
  strftime(buf, sizeof(buf), "%a, %d %b %Y %T GMT", ptm);
  this->response.modified = buf;
  return 0;
}

int HttpServer::updateDateInfo(){
  time_t cur_time;
  time(&cur_time);
  tm * ptm;
  ptm = gmtime (&cur_time);
  char buf[300] = {0};
  strftime(buf, sizeof(buf), "%a, %d %b %Y %T GMT", ptm);
  date = buf;
  return 0;
}

void HttpServer::configureResponse(int status_code){
  response.setStatus(status_code, this->status_codes[status_code]);
  response.setDefaultBody(this->status_codes[status_code]);
  updateDateInfo();
  response.setDate(date);
  response.setServer();
  response.setContentLength();
}

bool HttpServer::tookResource(){
  int code = this->loadResource();
  if ( code > 0 ) return true;

  if ( code == 0 ){
    this->configureResponse(403);
    return false;
  }

  this->configureResponse(404);
  return false;
}

void HttpServer::handleRequest(){
  int parse_code = request.parse(default_directory);
  string method = parse_code == 0 ? request.getMethod() : "ERROR";

  if ( method == "GET" || method == "HEAD" || method == "POST"
                                    || method == "PUT" || method == "DELETE"){

      int tp;
      if ( ( tp = cgi.loadResource(request) ) > -1 ){
        string t_path = cgi.temp_dir;
        t_path += "/";
        t_path += to_string(tp) + ".tmp";
        this->loadResource(t_path);
        cout << t_path << endl << endl;
        if ( unlink(t_path.c_str()) < 0 ){
          cout << "cannot delete file\n";
          perror(NULL);
        }
        cout << "Resource loaded!!!!!" << endl;
        this->configureResponse(200);
        this->response.setBody();
        return;
      }
  }

  if ( method == "GET" ){
    if (!this->tookResource()){
      this->response.setContentType("html");
      this->response.setBody();
      return;
    }
    this->configureResponse(200);
    this->response.setContentType(this->request.getPath());
    this->response.setLastModified();
  } else if ( method == "HEAD" ){
    if (!this->tookResource()){
      this->response.setContentType("html");
      this->response.setBody();
      return;
    }
    this->response.setStatus(200, this->status_codes[200]);
    this->response.setLastModified();
  } else if ( method == "POST" || method == "PUT" || method == "DELETE"){
    this->configureResponse(501);
    this->response.setContentType("html");
    this->response.setAllow();
  } else {
    this->response.setContentType("html");
    this->configureResponse(400);
  }
  this->response.setBody();
}
