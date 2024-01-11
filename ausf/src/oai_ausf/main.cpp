/*
 * Copyright (c) 2017 Sprint
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ausf-api-server.h"
#include "ausf-http2-server.h"
#include "ausf_app.hpp"
#include "ausf_config.hpp"
#include "ausf_config_yaml.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "pid_file.hpp"

#include "pistache/endpoint.h"
#include "pistache/http.h"
#include "pistache/router.h"

#include <iostream>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>  // srand
#include <thread>
#include <unistd.h>  // get_pid(), pause()

using namespace oai::ausf::app;
using namespace util;
using namespace std;

using namespace oai::config;

ausf_config ausf_cfg;
ausf_app* ausf_app_inst              = nullptr;
AUSFApiServer* api_server            = nullptr;
ausf_http2_server* ausf_api_server_2 = nullptr;

std::unique_ptr<ausf_config_yaml> ausf_cfg_yaml = nullptr;
//------------------------------------------------------------------------------
void my_app_signal_handler(int s) {
  std::cout << "Caught signal " << s << std::endl;
  Logger::system().startup("exiting");
  std::cout << "Freeing Allocated memory..." << std::endl;
  if (api_server) {
    api_server->shutdown();
    delete api_server;
    api_server = nullptr;
  }
  std::cout << "AUSF API Server memory done" << std::endl;

  if (ausf_app_inst) {
    delete ausf_app_inst;
    ausf_app_inst = nullptr;
  }

  std::cout << "AUSF APP memory done" << std::endl;
  std::cout << "Freeing allocated memory done" << std::endl;

  exit(0);
}

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
  srand(time(NULL));

  // Command line options
  if (!Options::parse(argc, argv)) {
    std::cout << "Options::parse() failed" << std::endl;
    return 1;
  }

  // Logger
  Logger::init("ausf", Options::getlogStdout(), Options::getlogRotFilelog());
  Logger::ausf_server().startup("Options parsed");

  std::signal(SIGTERM, my_app_signal_handler);
  std::signal(SIGINT, my_app_signal_handler);

  // Event subsystem
  ausf_event ev;

  // Config
  std::string conf_file_name = Options::getlibconfigConfig();
  std::string file_ext       = ".conf";
  if (conf_file_name.find(file_ext) != std::string::npos) {
    Logger::ausf_server().debug(
        "Parsing the configuration file, file type CONF.");
    ausf_cfg.load(conf_file_name);
    Logger::set_level(ausf_cfg.log_level);
    ausf_cfg.display();
  } else {
    // By default, considering the config file as yaml
    Logger::ausf_server().debug(
        "Parsing the configuration file, file type YAML.");
    ausf_cfg_yaml = std::make_unique<ausf_config_yaml>(
        conf_file_name, Options::getlogStdout(), Options::getlogRotFilelog());
    if (!ausf_cfg_yaml->init()) {
      Logger::ausf_server().error("Reading the configuration failed. Exiting.");
      return 1;
    }
    ausf_cfg_yaml->pre_process();
    ausf_cfg_yaml->display();
    // Convert from YAML to internal structure
    ausf_cfg_yaml->to_ausf_config(ausf_cfg);
  }

  // AUSF application layer
  ausf_app_inst = new ausf_app(Options::getlibconfigConfig(), ev);

  // Task Manager
  task_manager tm(ev);
  std::thread task_manager_thread(&task_manager::run, &tm);

  // PID file
  string pid_file_name =
      get_exe_absolute_path(ausf_cfg.pid_dir, ausf_cfg.instance);
  if (!is_pid_file_lock_success(pid_file_name.c_str())) {
    Logger::ausf_server().error(
        "Lock PID file %s failed\n", pid_file_name.c_str());
    exit(-EDEADLK);
  }

  FILE* fp             = NULL;
  std::string filename = fmt::format("/tmp/ausf_{}.status", getpid());
  fp                   = fopen(filename.c_str(), "w+");
  fprintf(fp, "STARTED\n");

  if (!ausf_cfg.use_http2) {
    // AUSF Pistache API server (HTTP1)
    Pistache::Address addr(
        std::string(inet_ntoa(*((struct in_addr*) &ausf_cfg.sbi.addr4))),
        Pistache::Port(ausf_cfg.sbi.port));
    api_server = new AUSFApiServer(addr, ausf_app_inst);
    api_server->init(2);
    std::thread ausf_manager(&AUSFApiServer::start, api_server);
    ausf_manager.join();
  } else {
    // AUSF NGHTTP API server (HTTP2)
    ausf_api_server_2 = new ausf_http2_server(
        conv::toString(ausf_cfg.sbi.addr4), ausf_cfg.sbi_http2_port,
        ausf_app_inst);
    std::thread ausf_http2_manager(
        &ausf_http2_server::start, ausf_api_server_2);
    ausf_http2_manager.join();
  }

  task_manager_thread.join();

  fflush(fp);
  fclose(fp);

  pause();
  return 0;
}
