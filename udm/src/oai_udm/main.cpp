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

#include <signal.h>
#include <stdint.h>
#include <stdlib.h>  // srand
#include <unistd.h>  // get_pid(), pause()

#include <iostream>
#include <thread>

#include "logger.hpp"
#include "options.hpp"
#include "pid_file.hpp"
#include "pistache/endpoint.h"
#include "pistache/http.h"
#include "pistache/router.h"
#include "udm-api-server.h"
#include "udm-http2-server.h"
#include "udm_app.hpp"
#include "udm_config.hpp"
#include "udm_config_yaml.hpp"

using namespace oai::udm::app;
using namespace oai::udm::config;
using namespace oai::config;
using namespace util;
using namespace std;

udm_config udm_cfg;
udm_app* udm_app_inst              = nullptr;
UDMApiServer* api_server           = nullptr;
udm_http2_server* udm_api_server_2 = nullptr;

std::unique_ptr<udm_config_yaml> udm_cfg_yaml;
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
  std::cout << "UDM API Server memory done" << std::endl;

  if (udm_app_inst) {
    delete udm_app_inst;
    udm_app_inst = nullptr;
  }

  std::cout << "UDM APP memory done" << std::endl;
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
  Logger::init("udm", Options::getlogStdout(), Options::getlogRotFilelog());
  Logger::udm_server().startup("Options parsed");

  std::signal(SIGTERM, my_app_signal_handler);
  std::signal(SIGINT, my_app_signal_handler);

  // Event subsystem
  udm_event ev;

  std::string conf_file_name = Options::getlibconfigConfig();
  std::string file_ext       = ".conf";
  if (conf_file_name.find(file_ext) != std::string::npos) {
    Logger::udm_server().debug(
        "Parsing the configuration file, file type CONF.");
    udm_cfg.load(conf_file_name);
    Logger::set_level(udm_cfg.log_level);
    udm_cfg.display();
  } else {
    // By default, considering the config file as yaml
    Logger::system().debug("Parsing the configuration file, file type YAML.");
    udm_cfg_yaml = std::make_unique<udm_config_yaml>(
        conf_file_name, Options::getlogStdout(), Options::getlogRotFilelog());
    if (!udm_cfg_yaml->init()) {
      Logger::udm_server().error("Reading the configuration failed. Exiting.");
      return 1;
    }
    udm_cfg_yaml->pre_process();
    udm_cfg_yaml->display();
    // Convert from YAML to internal structure
    udm_cfg_yaml->to_udm_config(udm_cfg);
  }

  // UDM application layer
  udm_app_inst = new udm_app(Options::getlibconfigConfig(), ev);

  // Task Manager
  task_manager tm(ev);
  std::thread task_manager_thread(&task_manager::run, &tm);

  // PID file
  string pid_file_name =
      get_exe_absolute_path(udm_cfg.pid_dir, udm_cfg.instance);
  if (!is_pid_file_lock_success(pid_file_name.c_str())) {
    Logger::udm_server().error(
        "Lock PID file %s failed\n", pid_file_name.c_str());
    exit(-EDEADLK);
  }

  FILE* fp             = NULL;
  std::string filename = fmt::format("/tmp/udm_{}.status", getpid());
  fp                   = fopen(filename.c_str(), "w+");
  fprintf(fp, "STARTED\n");

  if (!udm_cfg.use_http2) {
    // UDM Pistache API server (HTTP1)
    Pistache::Address addr(
        std::string(inet_ntoa(*((struct in_addr*) &udm_cfg.sbi.addr4))),
        Pistache::Port(udm_cfg.sbi.port));
    api_server = new UDMApiServer(addr, udm_app_inst);
    api_server->init(2);
    std::thread udm_manager(&UDMApiServer::start, api_server);
    udm_manager.join();
  } else {
    // UDM NGHTTP API server (HTTP2)
    udm_api_server_2 = new udm_http2_server(
        conv::toString(udm_cfg.sbi.addr4), udm_cfg.sbi_http2_port,
        udm_app_inst);
    std::thread udm_http2_manager(&udm_http2_server::start, udm_api_server_2);
    udm_http2_manager.join();
  }

  task_manager_thread.join();

  fflush(fp);
  fclose(fp);

  pause();
  return 0;
}
