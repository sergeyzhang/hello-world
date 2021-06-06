#ifndef TASK_POOL_H
#define TASK_POOL_H

#include <iostream>
#include <fstream>
#include <regex>
#include "../utils/Help.h"
#include "AbstractActivityRecognition.h"
#include "AbstractSystemInformation.h"

class TaskPool {
 public:
  int execute_file_watcher_task();
  int execute_cpu_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_memory_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_disk_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_account_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_pid_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_registry_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_ports_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_internet_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_process_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_firewall_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_system_logs_task(
      const AbstractSystemInformation* system_information_collector);
  int execute_logon_status_task(
      const AbstractSystemInformation* system_information_collector);
  // activity recognition
  int execute_activity_recognition_task1(
      const AbstractActivityRecognition* activity_recognition_collector);
  int execute_activity_recognition_task2(
      const AbstractActivityRecognition* activity_recognition_collector);
  int execute_activity_recognition_task3(
      const AbstractActivityRecognition* activity_recognition_collector);
};

#endif
