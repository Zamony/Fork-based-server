#include <string>
#include <map>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "CGIComponentClass.h"

bool ends_with(std::string const & value, std::string const & ending){
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

CGIComponent::CGIComponent(std::string sname, int sport, std::string sip){
  srv_name = sname;
  srv_port = sport;
  srv_ip = sip;

  //Read installation_dir from config
  //...

  //readHandlersFromConfig(handlers), KISS
  handlers[".php"] = "php_bin";
  handlers[".asp"] = "asp_bin";
  handlers[".mjs"] = "mjs_bin";
}

CGIComponent::~CGIComponent(){
  for (int i = 0; i < VARIABLES_COUNT; i++) free(env[i]);
}

int CGIComponent::loadResource(const HttpRequest & req){
  cout << req.getPathMain() << " " << req.getPathQuery() << endl;
  for (auto const& x : handlers){
    if (!ends_with(req.getPathMain(), x.first)) continue;
    return launchInterpreter(req, x.second);
  }
  return -1;
};

void CGIComponent::setEnvVariables(const HttpRequest & req, std::string program){

  string temp = "GATEWAY_INTERFACE=CGI/1.1";
  env[0] = strdup(temp.c_str());

  temp = "REMOTE_ADDR=";
  temp = temp + req.client_ip;
  env[1] = strdup(temp.c_str());

  temp = "REMOTE_PORT=";
  temp += to_string(req.client_port);
  env[2] = strdup(temp.c_str());


  temp = "QUERY_STRING=";
  temp += req.getPathQuery();
  env[3] = strdup(temp.c_str());

  temp = "SERVER_ADDR=";
  temp += srv_ip;
  env[4] = strdup(temp.c_str());

  temp = "SERVER_NAME=";
  temp += srv_name;
  env[5] = strdup(temp.c_str());

  temp = "SERVER_PORT=";
  temp += to_string(srv_port);
  env[6] = strdup(temp.c_str());

  temp = "SERVER_PROTOCOL=HTTP/1.1";
  env[7] = strdup(temp.c_str());

  temp = "SERVER_SOFTWARE=lserv/0.01";
  env[8] = strdup(temp.c_str());

  temp = "SCRIPT_NAME=";
  temp += dir;
  temp += "/";
  temp += program;
  env[9] = strdup(temp.c_str());

  temp = "SCRIPT_FILENAME=";
  temp += installation_dir;
  temp += "/";
  temp += dir;
  temp += "/";
  temp += program;
  env[10] = strdup(temp.c_str());

  temp = "DOCUMENT_ROOT=";
  temp += installation_dir;
  env[11] = strdup(temp.c_str());

  temp = "HTTP_USER_AGENT=";
  temp += req.getUserAgent();
  env[12] = strdup(temp.c_str());

  temp = "HTTP_REFERER=";
  temp += req.getReferer();
  env[13] = strdup(temp.c_str());

  temp = "REQUEST_METHOD=";
  temp += req.getMethod();
  env[14] = strdup(temp.c_str());

}

int CGIComponent::launchInterpreter(const HttpRequest & req, std::string program){
  int pid, status;

  setEnvVariables(req, program);

  int mypid = getpid();
  std::string temp_filename = temp_dir;
  temp_filename += "/";
  temp_filename += to_string(getpid()) + ".tmp";
  string path = dir;
  path += "/";
  path += program;

  string script_path = req.getPath();

  int fout = open(temp_filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777 );
  cout << "temp filename is " << temp_filename << endl;

  int fin;
  string method = req.getMethod();
  bool protected_send = method == "PUT" || method == "POST";
  string tf;
  if ( protected_send ){
    cout << "protected send\n";
    tf = temp_dir;
    tf += "/";
    tf += to_string(getpid());
    tf += "in.tmp";
    cout << tf << endl;
    fin = open(tf.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0777 );
    for (int i = 0; i < VARIABLES_COUNT; i++){
      write(fin, env[i], strlen(env[i]));
      write(fin, "\n", 1);
    }
    close(fin);
    fin = open(tf.c_str(), O_RDONLY);
    cout << "loop end\n";
  }

  cout << "Program name is: " << program << endl;

  if ( (pid = fork()) ){
    close(fin);
    waitpid(pid, &status, 0);
    close(fout);
    unlink(tf.c_str());
  } else {
    if ( protected_send ){
      dup2(fin, 0);
      close(fin);
    }
    dup2(fout, 1);
    close(fout);

    if (protected_send)
      execl(path.c_str(), program.c_str(), script_path.c_str(), NULL);

    if ( execle(path.c_str(), program.c_str(), script_path.c_str(), NULL, env) < 0 ){
      cerr << "Exec failed\n";
      perror(NULL);
    }
  }
  return mypid;
}
