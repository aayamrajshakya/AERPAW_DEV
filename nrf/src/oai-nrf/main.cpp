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

#include "logger.hpp"
#include "nrf-api-server.h"
#include "nrf-http2-server.h"
#include "nrf_app.hpp"
#include "nrf_client.hpp"
#include "options.hpp"
#include "pid_file.hpp"
#include "conversions.hpp"

#include "pistache/endpoint.h"
#include "pistache/http.h"
#include "pistache/router.h"

#include <signal.h>
#include <stdint.h>
#include <stdlib.h>  // srand
#include <unistd.h>  // get_pid(), pause()
#include <iostream>
#include <thread>
#include "nrf_config.hpp"

using namespace oai::nrf::app;
using namespace util;
using namespace std;
using namespace oai::config::nrf;

nrf_app* nrf_app_inst = nullptr;
std::unique_ptr<nrf_config> nrf_cfg;
NRFApiServer* api_server           = nullptr;
nrf_http2_server* nrf_api_server_2 = nullptr;

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
  std::cout << "NRF API Server memory done" << std::endl;

  if (nrf_app_inst) {
    delete nrf_app_inst;
    nrf_app_inst = nullptr;
  }

  std::cout << "NRF APP memory done" << std::endl;
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
  Logger::init("nrf", Options::getlogStdout(), Options::getlogRotFilelog());
  Logger::nrf_app().startup("Options parsed");

  std::signal(SIGTERM, my_app_signal_handler);
  std::signal(SIGINT, my_app_signal_handler);

  // Config
  nrf_cfg = std::make_unique<nrf_config>(
      Options::getlibconfigConfig(), Options::getlogStdout(),
      Options::getlogRotFilelog());
  if (!nrf_cfg->init()) {
    nrf_cfg->display();
    Logger::system().error("Reading the configuration failed. Exiting");
    return 1;
  }
  nrf_cfg->display();

  // Event subsystem
  nrf_event ev;

  // NRF application layer
  nrf_app_inst = new nrf_app(Options::getlibconfigConfig(), ev);

  // Task Manager
  task_manager tm(ev);
  std::thread task_manager_thread(&task_manager::run, &tm);

  // PID file
  // Currently hard-coded value. TODO: add as config option.
  string pid_file_name = get_exe_absolute_path("/var/run", nrf_cfg->instance);
  if (!is_pid_file_lock_success(pid_file_name.c_str())) {
    Logger::nrf_app().error("Lock PID file %s failed\n", pid_file_name.c_str());
    exit(-EDEADLK);
  }

  if (nrf_cfg->get_http_version() == 1) {
    // NRF Pistache API server (HTTP1)
    Pistache::Address addr(
        std::string(inet_ntoa(
            *((struct in_addr*) &nrf_cfg->local().get_sbi().get_addr4()))),
        Pistache::Port(nrf_cfg->local().get_sbi().get_port()));
    api_server = new NRFApiServer(addr, nrf_app_inst);
    api_server->init(2);
    std::thread nrf_manager(&NRFApiServer::start, api_server);
    nrf_manager.join();
  } else if (nrf_cfg->get_http_version() == 2) {
    // NRF NGHTTP API server (HTTP2)
    nrf_api_server_2 = new nrf_http2_server(
        conv::toString(nrf_cfg->local().get_sbi().get_addr4()),
        nrf_cfg->local().get_sbi().get_port(), nrf_app_inst);
    std::thread nrf_http2_manager(&nrf_http2_server::start, nrf_api_server_2);
    nrf_http2_manager.join();
  }

  FILE* fp             = NULL;
  std::string filename = fmt::format("/tmp/nrf_{}.status", getpid());
  fp                   = fopen(filename.c_str(), "w+");
  fprintf(fp, "STARTED\n");
  fflush(fp);
  fclose(fp);

  pause();
  return 0;
}
