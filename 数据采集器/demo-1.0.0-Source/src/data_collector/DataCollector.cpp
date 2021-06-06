#include "../../include/data_collector/DataCollector.h"

#include "../../include/data_collector/AbstractSystemInformation.h"
#include "../../include/data_collector/TaskPool.h"

DataCollector::DataCollector() {
  system_information_ = new ConcreteSystemInformation();
  activity_recognition_ = new ConcreteActivityRecognition();
}

DataCollector::~DataCollector() {
  delete system_information_;
  system_information_ = nullptr;

  delete activity_recognition_;
  activity_recognition_ = nullptr;
}

int DataCollector::run() {
  assert(system_information_);
  assert(activity_recognition_);
  std::cout << "=================================================\n";
  std::cout << "=================System Info=====================\n";
  std::cout << "=================================================\n";
  TaskPool tasks;
  tasks.execute_cpu_task(system_information_);
  tasks.execute_memory_task(system_information_);
  tasks.execute_disk_task(system_information_);
  tasks.execute_account_task(system_information_);
  //tasks.execute_pid_task(system_information_);
  //tasks.execute_system_logs_task(system_information_);
  //tasks.execute_ports_task(system_information_);
  //tasks.execute_internet_task(system_information_);
  //tasks.execute_process_task(system_information_);
  //tasks.execute_registry_task(system_information_);
  //tasks.execute_firewall_task(system_information_);
  //tasks.execute_logon_status_task(system_information_);
  //tasks.execute_file_watcher_task(); 

  std::cout << "=================================================\n";
  std::cout << "==================Activity Info==================\n";
  std::cout << "=================================================\n";
  tasks.execute_activity_recognition_task1(activity_recognition_);
  tasks.execute_activity_recognition_task2(activity_recognition_);
  tasks.execute_activity_recognition_task3(activity_recognition_);

  return 0;
}