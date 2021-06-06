#include "../../include/utils/Client.h"
#include "../../include/data_collector/DataCollector.h"
#include "../../include/data_transmitter/DataTransmitter.h"

int Client::run_data_collector() {
  DataCollector data_collector;
  int ret = data_collector.run();
  if (ret != 0) {
    std::cout << "ERROR: run_data_coolector() failed.\n";
    return -1;
  }
  data_collection_succeed_ = true;
  return 0;
}

int Client::run_data_transmitter() {
  std::cout << "=================================================\n";
  std::cout << "=================Data Transmission===============\n";
  std::cout << "=================================================\n";
  if (!data_collection_succeed_) {
    std::cout << "WARN: Run data collector first.\n";
    return -1;
  }
  DataTransmitter data_transmitter;
  int ret = data_transmitter.run();
  if (ret != 0) {
    std::cout << "WARN: transmission failed.\n" << std::endl;
    return -1;
  }
  std::cout << "INFO: transmission done.\n" << std::endl;
  return 0;
}