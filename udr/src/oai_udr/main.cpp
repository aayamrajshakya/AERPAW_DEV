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

#include "conversions.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "pid_file.hpp"
#include "udr-api-server.h"
#include "udr-http2-server.h"
#include "udr_app.hpp"
#include "udr_nrf.hpp"
#include "udr_config.hpp"
#include "udr_config_yaml.hpp"

using namespace util;
using namespace std;
using namespace oai::udr::app;
using namespace oai::udr::config;
using namespace oai::config;

udr_config udr_cfg;
udr_app* udr_app_inst          = nullptr;
udr_nrf* udr_nrf_inst          = nullptr;
UDRApiServer* http_server1     = nullptr;
udr_http2_server* http_server2 = nullptr;

std::unique_ptr<udr_config_yaml> udr_cfg_yaml;

//------------------------------------------------------------------------------
void my_app_signal_handler(int s) {
  std::cout << "Caught signal " << s << std::endl;
  Logger::system().startup("exiting");
  std::cout << "Freeing Allocated memory..." << std::endl;
  std::cout << "Shutting down HTTP servers..." << std::endl;
  if (http_server1) {
    http_server1->shutdown();
    delete http_server1;
    http_server1 = nullptr;
  }
  if (http_server2) {
    http_server2->stop();
    delete http_server2;
    http_server2 = nullptr;
  }

  if (udr_app_inst) {
    delete udr_app_inst;
    udr_app_inst = nullptr;
  }
  std::cout << "UDR APP memory done" << std::endl;
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
  Logger::init("udr", Options::getlogStdout(), Options::getlogRotFilelog());
  Logger::udr_server().startup("Options parsed");

  std::signal(SIGTERM, my_app_signal_handler);
  std::signal(SIGINT, my_app_signal_handler);

  // Event subsystem
  udr_event ev;

  // Config
  std::string conf_file_name = Options::getlibconfigConfig();
  std::string file_ext       = ".conf";
  if (conf_file_name.find(file_ext) != std::string::npos) {
    Logger::udr_server().debug(
        "Parsing the configuration file, file type CONF.");
    udr_cfg.load(conf_file_name);
    Logger::set_level(udr_cfg.log_level);
    udr_cfg.display();
  } else {
    // By default, considering the config file as yaml
    Logger::system().debug("Parsing the configuration file, file type YAML.");
    udr_cfg_yaml = std::make_unique<udr_config_yaml>(
        conf_file_name, Options::getlogStdout(), Options::getlogRotFilelog());
    if (!udr_cfg_yaml->init()) {
      Logger::udr_server().error("Reading the configuration failed. Exiting.");
      return 1;
    }
    udr_cfg_yaml->pre_process();
    udr_cfg_yaml->display();
    // Convert from YAML to internal structure
    udr_cfg_yaml->to_udr_config(udr_cfg);
  }

  // UDR application layer
  udr_app_inst = new udr_app(Options::getlibconfigConfig(), ev);

  // Task Manager
  task_manager tm(ev);
  std::thread task_manager_thread(&task_manager::run, &tm);

  // UDR NRF
  udr_nrf_inst = new udr_nrf(ev);
  std::thread udr_nrf_manager(&udr_nrf::start, udr_nrf_inst);

  // PID file
  string pid_file_name =
      get_exe_absolute_path(udr_cfg.pid_dir, udr_cfg.instance);
  if (!is_pid_file_lock_success(pid_file_name.c_str())) {
    Logger::udr_server().error(
        "Lock PID file %s failed\n", pid_file_name.c_str());
    exit(-EDEADLK);
  }

  FILE* fp             = NULL;
  std::string filename = fmt::format("/tmp/udr_{}.status", getpid());
  fp                   = fopen(filename.c_str(), "w+");
  fprintf(fp, "STARTED\n");

  if (!udr_cfg.use_http2) {
    // UDR Pistache API server (HTTP1)
    Pistache::Address addr(
        std::string(inet_ntoa(*((struct in_addr*) &udr_cfg.nudr.addr4))),
        Pistache::Port(udr_cfg.nudr.port));

    http_server1 = new UDRApiServer(addr, udr_app_inst);
    http_server1->init(2);
    std::thread udr_http1_manager(&UDRApiServer::start, http_server1);
    udr_http1_manager.join();
  } else {
    // UDR NGHTTP API server (HTTP2)
    http_server2 = new udr_http2_server(
        conv::toString(udr_cfg.nudr.addr4), udr_cfg.nudr_http2_port,
        udr_app_inst);
    std::thread udr_http2_manager(&udr_http2_server::start, http_server2);
    udr_http2_manager.join();
  }

  task_manager_thread.join();
  udr_nrf_manager.join();

  fflush(fp);
  fclose(fp);
  pause();
  return 0;
}
