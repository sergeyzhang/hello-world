#include "../../include/data_collector/TaskPool.h"

int TaskPool::execute_cpu_task(
    const AbstractSystemInformation* system_information_collector) {
  const CPU* cpu = system_information_collector->create_cpu_information();
  if (!cpu) {
    std::cout << "Error! Failed to get cpu info!\n";
    return -1;
  }
  cpu->get_cpu_use_rate();
  delete (cpu);
  cpu = nullptr;
  return 0;
}

int TaskPool::execute_memory_task(
    const AbstractSystemInformation* system_information_collector) {
  const Memory* memory =
      system_information_collector->create_memory_information();
  if (!memory) {
    std::cout << "Error! Failed to get memory info!\n";
    return -1;
  }
  memory->get_memory_volume();
  delete (memory);
  memory = nullptr;
  return 0;
}

int TaskPool::execute_disk_task(
    const AbstractSystemInformation* system_information_collector) {
  const Disk* disk = system_information_collector->create_disk_information();
  if (!disk) {
    std::cout << "Error! Failed to get disk info!\n";
    return -1;
  }
  disk->get_disk_info();
  delete (disk);
  disk = nullptr;
  return 0;
}

int TaskPool::execute_account_task(
    const AbstractSystemInformation* system_information_collector) {
  const Account* account =
      system_information_collector->create_account_information();
  if (!account) {
    std::cout << "Error! Failed to get name info!\n";
    return -1;
  }
  account->get_account_info();
  delete (account);
  account = nullptr;
  return 0;
}

int TaskPool::execute_pid_task(
    const AbstractSystemInformation* system_information_collector) {
  const PID* pid = system_information_collector->create_pid_information();
  if (!pid) {
    std::cout << "Error! Failed to get pid info!\n";
    return -1;
  }
  pid->get_pid_info();
  delete (pid);
  pid = nullptr;
  return 0;
}

int TaskPool::execute_system_logs_task(
    const AbstractSystemInformation* system_information_collector) {
  SystemLogs* system_logs =
      system_information_collector->create_system_logs_information();
  if (!system_logs) {
    std::cout << "Error! Failed to get system_logs info!\n";
    return -1;
  }
  system_logs->get_system_logs_info();
  delete (system_logs);
  system_logs = nullptr;
  return 0;
}

int TaskPool::execute_ports_task(
    const AbstractSystemInformation* system_information_collector) {
  const Ports* ports = system_information_collector->create_ports_information();
  if (!ports) {
    std::cout << "Error! Failed to get ports info!\n";
    return -1;
  }
  ports->get_ports_info();
  delete (ports);
  ports = nullptr;
  return 0;
}

int TaskPool::execute_internet_task(
    const AbstractSystemInformation* system_information_collector) {
  const Internet* internet =
      system_information_collector->create_internet_information();
  if (!internet) {
    std::cout << "Error! Failed to get ports info!\n";
    return -1;
  }
  internet->get_internet_info();
  delete (internet);
  internet = nullptr;
  return 0;
}

int TaskPool::execute_process_task(
    const AbstractSystemInformation* system_information_collector) {
  Process* process = system_information_collector->create_process_information();
  if (!process) {
    std::cout << "Error! Failed to get process info!\n";
    return -1;
  }
  process->actions_on_process();
  delete (process);
  process = nullptr;
  return 0;
}

int TaskPool::execute_registry_task(
    const AbstractSystemInformation* system_information_collector) {
  const Registry* registry =
      system_information_collector->create_registry_information();
  if (!registry) {
    std::cout << "Error! Failed to get registry info!\n";
    return -1;
  }
  registry->get_registry_info();
  delete (registry);
  registry = nullptr;
  return 0;
}

int TaskPool::execute_firewall_task(
    const AbstractSystemInformation* system_information_collector) {
  Firewall* firewall =
      system_information_collector->create_firewall_information();
  if (!firewall) {
    std::cout << "Error! Failed to get process info!\n";
    return -1;
  }
  firewall->get_firewall_info();
  delete (firewall);
  firewall = nullptr;
  return 0;
}

int TaskPool::execute_logon_status_task(
    const AbstractSystemInformation* system_information_collector) {
  LogonStatus* logon_status =
      system_information_collector->create_logon_status_information();
  if (!logon_status) {
    std::cout << "Error! Failed to get system_logs info!\n";
    return -1;
  }
  logon_status->get_logon_status_info();
  delete (logon_status);
  logon_status = nullptr;
  return 0;
}

int TaskPool::execute_file_watcher_task() {
  std::cout << "File Watcher: \n";
#ifndef __linux__
  std::string script_path = ".\\FileWatcher.exe";
  std::ifstream file(script_path);
  if (!file.is_open()) {
    std::cout << "INFO: file watcher not exist.\n";
    return -1;
  }
  system("start .\\FileWatcher.exe");
#else
  pid_t child_pid = fork();  // consume time+space more than thread, but 1 safe;
                             // 2 no mutual communication
  if (child_pid == -1) {
    std::cout << "ERROR: fork() failed.\n";
    exit(EXIT_FAILURE);
  } else if (child_pid > 0) {
    int status;
    waitpid(child_pid, &status, 0);
  } else {
    int fd = inotify_init();
    if (fd < 0) {
      std::cout << "ERROR: inotify_init failed.\n";
      exit(EXIT_FAILURE);
    }
    const char* path_to_be_watched = "/home/xian/Downloads";
    int wd = inotify_add_watch(fd, path_to_be_watched, IN_ALL_EVENTS);
    if (wd < 0) {
      std::cout << "ERROR: inotify_add_watch() failed.\n";
      exit(EXIT_FAILURE);
    }
    std::cout << "INFO: watching " << path_to_be_watched << "\n";
    const int FILE_NAME_LEN = 16;
    const int MAX_EVENTS = 1024;
    const int EVENT_SIZE = sizeof(inotify_event);  // size of one event
    const int BUF_LEN = MAX_EVENTS * (EVENT_SIZE + FILE_NAME_LEN);
    while (1) {
      int i = 0;
      int length = -1;
      char buffer[BUF_LEN];
      length = read(fd, buffer, BUF_LEN);
      while (i < length) {
        struct inotify_event* event = (struct inotify_event*)&buffer[i];
        if (event->len) {
          if (event->mask & IN_CREATE) {
            if (event->mask & IN_ISDIR) {
              std::cout << "INFO: directory " << event->name
                        << " was created.\n";
            } else {
              std::cout << "INFO: file " << event->name << " was created.\n";
            }
          } else if (event->mask & IN_DELETE) {
            if (event->mask & IN_ISDIR) {
              std::cout << "INFO: directory " << event->name
                        << " was deleted.\n";
            } else {
              std::cout << "INFO: file " << event->name << " was deleted.\n";
            }
          } else if (event->mask & IN_MODIFY) {
            if (event->mask & IN_ISDIR) {
              std::cout << "INFO: directory " << event->name
                        << " was modified.\n";
            } else {
              std::cout << "INFO: file " << event->name << " was modified.\n";
            }
          }
        }
        i += EVENT_SIZE + event->len;
      }
    }
    exit(EXIT_SUCCESS);  // don't forget
  }
#endif
  return 0;
}

// activity recognition tasks
int TaskPool::execute_activity_recognition_task1(
    const AbstractActivityRecognition* activity_recognition_collector) {
  const Activity1* activity1 =
      activity_recognition_collector->create_activity1_information();
  int ret = activity1->get_activity1_data();
  if (ret != 0) {
    std::cout << "Error! get_activity1_data() failed.\n";
    return -1;
  }
  delete (activity1);
  activity1 = NULL;
  return 0;
}

int TaskPool::execute_activity_recognition_task2(
    const AbstractActivityRecognition* activity_recognition_collector) {
  const Activity2* activity2 =
      activity_recognition_collector->create_activity2_information();
  int ret = activity2->get_activity2_data();
  if (ret != 0) {
    std::cout << "Error! get_activity2_data() failed.\n";
    return -1;
  }
  delete (activity2);
  activity2 = NULL;
  return 0;
}

int TaskPool::execute_activity_recognition_task3(
    const AbstractActivityRecognition* activity_recognition_collector) {
  const Activity3* activity3 =
      activity_recognition_collector->create_activity3_information();
  int ret = activity3->get_activity3_data();
  if (ret != 0) {
    std::cout << "Error! get_activity3_data() failed.\n";
    return -1;
  }
  delete (activity3);
  activity3 = NULL;
  return 0;
}

/*
 * todo: safe than system(exe)
 */
#if (0)
int TaskPool::execute_file_watcher_task() {
  std::string exe_path = ".\\FileWatcher.exe";
  std::ifstream file(exe_path);
  if (!file.is_open()) {
    std::cout << "    file watcher not exist.\n";
    return -1;
  }
  std::cout << "    file watcher exists.\n";
  STARTUPINFO startup_info{};  // don't: = {} which is copy-initialization
  PROCESS_INFORMATION process_info{};

  // set the size of the structures
  startup_info.cb = sizeof(startup_info);
  LPCSTR app_path = ".\\FileWatcher.exe";
  // start the program up
  CreateProcess(app_path,       // EXE path
                nullptr,        // LPSTR: command line, e.g. argv[1]
                nullptr,        // Process handle not inheritable
                nullptr,        // Thread handle not inheritable
                false,          // Set handle inheritance to FALSE
                0,              // No creation flags
                nullptr,        // Use parent's environment block
                nullptr,        // Use parent's starting directory
                &startup_info,  // Pointer to STARTUPINFO structure
                &process_info   // Pointer to PROCESS_INFORMATION structure
  );
  CloseHandle(process_info.hProcess);
  CloseHandle(process_info.hThread);
  return 0;
}
#endif
