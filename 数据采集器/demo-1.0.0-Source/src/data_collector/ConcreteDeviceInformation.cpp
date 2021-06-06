#include "../../include/data_collector/ConcreteDeviceInformation.h"

int ConcreteCpuInformation::get_cpu_use_rate() const {
  CpuInfo cpu_info;
  std::cout << "CPU Usage: " << cpu_info.get_result() << "%\n";
  return 0;
}

int ConcreteMemoryInformation::get_memory_volume() const {
  float mem_used = 0.0f;
#ifndef __linux__
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  GlobalMemoryStatusEx(&statex);
  mem_used = (float)statex.dwMemoryLoad;
#else
  std::string line, key;
  float multiplier = 0.001;
  float value;
  float mem_total;
  float mem_free;
  const std::string PROC_DIRECTORY{"/proc/"};
  const std::string MEM_INFO_FILE_NAME{"/meminfo"};
  std::ifstream my_stream(PROC_DIRECTORY + MEM_INFO_FILE_NAME);
  if (!my_stream.is_open()) {
    std::cout << "ERROR: file open failed.\n";
    return -1;
  }
  while (std::getline(my_stream, line)) {
    std::istringstream linestream(line);
    while (linestream >> key >> value) {
      if (key == "MemTotal:") {
        mem_total = value * multiplier;  // ok: convert divide by multiplication
      } else if (key == "MemFree:") {
        mem_free = value * multiplier;
      }
    }
  }
  mem_used = 100.0 * (mem_total - mem_free) / mem_total;  // ok: 100% format
  my_stream.close();  // ok: stop stream when finished instead of flush
#endif
  std::cout << "Memory Usage: " << mem_used << "%\n";
  return 0;
}

int ConcreteDiskInformation::get_disk_info() const {
  std::cout << "Disk Usage: ";
#ifndef __linux__
  long long int disk_free_volume;
  long long int disk_total_volume;
  long long int disk_available_volume;
  GetDiskFreeSpaceEx((LPCSTR) "C:\\",
                     (PULARGE_INTEGER)&disk_free_volume,  //---
                     (PULARGE_INTEGER)&disk_total_volume,
                     (PULARGE_INTEGER)&disk_available_volume);
  std::cout << "\n";
  std::cout << "INFO: total volume " << disk_total_volume * 0.000000001
            << " G\n";
  std::cout << "INFO: free volume: " << disk_free_volume * 0.000000001
            << " G\n";
  std::cout << "INFO: available volume: " << disk_available_volume * 0.000000001
            << " G\n";
#else
  struct statvfs stat {};
  const char* path = "/home";
  if (statvfs(path, &stat) != 0) {
    std::cout << "ERROR: statvfs() failed.\n";
    return -1;
  }
  float multiplier = 0.000000001f;
  std::cout << "/home: " << stat.f_bsize * stat.f_bavail * multiplier << " G\n";
#endif
  return 0;
}

int ConcreteAccountInformation::get_account_info() const {
  std::cout << "Account Info: \n";
  int ret = -1;
#ifndef __linux__
  const int BUFLEN = MAX_COMPUTERNAME_LENGTH + 1;
#else
  const int BUFLEN = 32;  // max name length in GNU-Linux
#endif
  char buffer[BUFLEN];
  std::fill(buffer, buffer + BUFLEN, 0);
  ret = gsl::ensures(buffer, sizeof(buffer) / sizeof(buffer[0]));
  if (ret != 0) {
    std::cout << "ERROR: std::fill failed.\n";
    return -1;
  }
#ifndef __linux__
  DWORD name_length = MAX_COMPUTERNAME_LENGTH + 1;
  GetComputerName((LPSTR)buffer, &name_length);  // check: may not safe
  std::string computer_name;
  computer_name.assign(buffer);
  std::string user_name;
  std::fill(buffer, buffer + BUFLEN, 0);
  ret = gsl::ensures(buffer, sizeof(buffer) / sizeof(buffer[0]));
  if (ret != 0) {
    std::cout << "ERROR: std::fill failed.\n";
    return -1;
  }
  GetUserName((LPSTR)buffer, &name_length);
  user_name.assign(buffer);
  std::cout << "INFO: computer name is " << computer_name << "\n";
  std::cout << "INFO: user     name is " << user_name << "\n";
  exec_concat_cmd(user_name);  // show user's info before below operation
  std::cout << "INFO: PLEASE READ BELOW CAREFULLY\n";
  add_account();
  delete_account();
#else
  ret = getlogin_r(buffer, BUFLEN - 1);
  if (ret != 0) {
    std::cout << "ERROR: getlogin_r() failed.\n";
    return -1;
  }
  std::cout << "INFO: user name is " << buffer << "\n";
  std::cout << "INFO: user id   is " << getuid() << "\n";
  pid_t child_pid = fork();  // consume time+space more than thread, but 1 safe;
                             // 2 no mutual communication
  if (child_pid == -1) {
    std::cout << "ERROR: fork() failed.\n";
    exit(EXIT_FAILURE);
  } else if (child_pid > 0) {
    int status;
    waitpid(child_pid, &status, 0);
  } else {
    std::cout << "INFO: PLEASE READ BELOW CAREFULLY\n";
    std::cout << "INFO: use 'cmd' to add or remove accounts\n";
    std::cout << "INFO: [1] add   : e.g. sudo useradd account_name\n";
    std::cout << "INFO: [2] remove: e.g. sudo userdel account_name\n";
    std::cout << "INFO: accounts available: " << gsl::exec_cmd("groups");
    std::string operation_mode, cmd;
    int ret1, ret2;
    ret1 = -1;
    ret2 = -1;
    bool flag1 = true;
    while (flag1) {
      std::cout << "INFO: input your choice: ";
      getline(std::cin, operation_mode);
      std::regex expression("\\s+");
      operation_mode = std::regex_replace(operation_mode, expression,
                                          " ");  // remove whitespace
      ret1 = operation_mode.compare("add");
      ret2 = operation_mode.compare("remove");
      if (ret1 == 0) {
        std::cout << "----- ADD enabled.\n";
        while (cmd.empty()) {
          std::cout << "INFO: Input Your cmd: ";
          getline(std::cin, cmd);
        }
        gsl::exec_cmd(cmd);
        std::cout
            << "INFO: new account added, use 'cat /etc/passwd' to check'\n";
        flag1 = false;
      } else if (ret2 == 0) {
        std::cout << "----- REMOVE enabled.\n";
        while (cmd.empty()) {
          std::cout << "INFO: Input Your cmd: ";
          getline(std::cin, cmd);
        }
        gsl::exec_cmd(cmd);
        std::cout << "INFO: account removed, use 'cat /etc/passwd' to check'\n";
        flag1 = false;
      } else {
        std::cout << "wrong input, try again.\n" << std::flush;
      }
    }
    exit(EXIT_SUCCESS);  // don't forget
  }
#endif
  return 0;
}

#ifndef __linux__
int ConcreteAccountInformation::add_account() const {
  USER_INFO_1 user_info;
  DWORD level, ret;
  level = 1;
  ret = 0;
  NET_API_STATUS status;
  LPWSTR argv1 = (LPWSTR)L"";
  LPWSTR argv2 = (LPWSTR)L"xyz";
  LPWSTR argv3 = (LPWSTR)L"123";
  user_info.usri1_name = argv2;
  user_info.usri1_password = argv3;
  user_info.usri1_priv = USER_PRIV_USER;
  user_info.usri1_home_dir = NULL;
  user_info.usri1_comment = NULL;
  user_info.usri1_flags = UF_SCRIPT;
  user_info.usri1_script_path = NULL;
  status = NetUserAdd(argv1, level, (LPBYTE)&user_info, &ret);
  if (status != NERR_Success) {
    std::cout << "ERROR: NetUserAdd() failed.\n";
    return -1;
  }
  std::cout << "INFO: account " << argv2 << " is added\n";
  return 0;
}

int ConcreteAccountInformation::delete_account() const {
  USER_INFO_1 user_info;
  DWORD level, ret;
  level = 1;
  ret = 0;
  NET_API_STATUS status;
  LPWSTR argv1 = (LPWSTR)L"";
  LPWSTR argv2 = (LPWSTR)L"xyz";
  LPWSTR argv3 = (LPWSTR)L"123";
  user_info.usri1_name = argv2;
  user_info.usri1_password = argv3;
  user_info.usri1_priv = USER_PRIV_USER;
  user_info.usri1_home_dir = NULL;
  user_info.usri1_comment = NULL;
  user_info.usri1_flags = UF_SCRIPT;
  user_info.usri1_script_path = NULL;
  status = NetUserDel(argv1, argv2);
  if (status != NERR_Success) {
    std::cout << "ERROR: NetUserDel() failed.\n";
    return -1;
  }
  std::cout << "INFO: account " << argv2 << " is deleted\n";
  return 0;
}

int ConcreteAccountInformation::exec_concat_cmd(const std::string& src) const {
  const int RESULT_SIZE = 5120;
  const int BUFFER_SIZE = 1024;
  const int COMMAND_SIZE = 100;
  char result[RESULT_SIZE], command[COMMAND_SIZE], buffer[BUFFER_SIZE];
  std::fill(result, result + sizeof(result) / sizeof(result[0]), 0);
  std::fill(command, command + sizeof(command) / sizeof(command[0]), 0);
  std::fill(buffer, buffer + sizeof(buffer) / sizeof(buffer[0]), 0);
  const int LEN = (int)src.size();  // ok: length(C-Style) and size(STL) same
  char* tmp = (char*)malloc(sizeof(char) * LEN + 1);
  const int CMD_HEAD_SIZE = 30;
  strcpy_s(tmp, LEN + 1, src.c_str());
  strcpy_s(command, CMD_HEAD_SIZE, "net users ");
  strcat_s(command, COMMAND_SIZE, tmp);
  FILE* pipe = _popen(command, "r");
  if (!pipe) {
    std::cout << "ERROR: _popen() failed.\n";
    return -1;
  }
  while (!feof(pipe)) {
    if (fgets(buffer, BUFFER_SIZE, pipe)) {
      strcat_s(result, RESULT_SIZE, buffer);
    }
  }
  std::cout << "INFO: execute concat command: \n";
  std::cout << result << "\n";
  free(tmp);
  tmp = nullptr;
  _pclose(pipe);
  return 0;
}

std::vector<std::string> ConcreteFirewallInformation::firewall_logs_list(
    const std::string& file_path) const {
  std::vector<std::string> names;
  std::string search_path = file_path + "*.*";  // don't: '//' or '\\'
  WIN32_FIND_DATA find_data;
  HANDLE hfind =
      FindFirstFile((LPCSTR)search_path.c_str(), &find_data);  // ----
  std::string ss;
  if (hfind != INVALID_HANDLE_VALUE) {
    while (FindNextFile(hfind, &find_data)) {
      /* read all real files in current folder,
       * delete '!' read other 2 default folder . and ..
       */
      if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        ss = find_data.cFileName;
        names.push_back(ss);
      }
    }
    FindClose(hfind);
  }
  return names;
}

int user_input() {
  std::cout << "INFO: PELEASE READ BELOW CAREFULLY*\n";
  std::cout << "INFO: all ongoing processes are list above\n";
  std::cout << "INFO: input <p> or <d> followed by 'ENTER'\n";
  std::cout << "INFO: to do <PAUSE> or <DELETE> operations\n";
  int ret1, ret2;
  bool flag1, flag2;
  flag1 = true;
  flag2 = true;
  std::string ss;
  while (flag1) {
    std::cout << "INFO: user input: ";
    getline(std::cin, ss);
    std::regex expression("\\s+");
    ss = std::regex_replace(ss, expression, " ");  // remove whitespace
    ret1 = ss.compare("p");
    ret2 = ss.compare("d");
    if (ret1 == 0) {
      std::cout << "INFO: PAUSE operation enabled.\n";
      suspend_enabled_ = true;
      flag1 = false;
    } else if (ret2 == 0) {
      std::cout << "    "
                << "INFO: DELETE operation enabled.\n";
      flag1 = false;
    } else {
      std::cout << "WARN: wrong input, try again.\n" << std::flush;
    }
  }
  int pid = -1;
  int count = 0;
  while (flag2) {
    std::cout << "INFO: input your pid number: ";
    getline(std::cin, ss);
    std::regex expression("\\s+");
    ss = std::regex_replace(ss, expression, " ");
    for (int i = 0; i < ss.length(); i++) {
      if (!isdigit(ss[i])) {
        count++;
      }
    }
    if (count == 0) {
      int value = 0;
      for (int i = 0; i < ss.length(); i++) {
        char ch = ss[i];
        if ('0' <= ch && ch <= '9') {
          value = value * 10 + (ch - '0');
        }
      }
      pid = value;  // not safe but have to
      flag2 = false;
      break;
    } else {
      count = 0;
      std::cout << "WARN: wrong input, try again.\n" << std::flush;
    }
  }
  std::cout << "INFO: your pid number is " << pid << std::endl;
  return pid;
}

int suspend_process(DWORD pid) {
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
  if (snapshot == INVALID_HANDLE_VALUE) {
    std::cout << "ERROR: CreateToolhelp32Snapshot() failed.\n";
    return -1;
  }
  THREADENTRY32 th32;
  th32.dwSize = sizeof(THREADENTRY32);
  if (!Thread32First(snapshot, &th32)) {
    CloseHandle(snapshot);
    std::cout << "ERROR: Thread32First() failed.\n";
    return -1;
  }
  while (Thread32Next(snapshot, &th32)) {
    if (th32.th32OwnerProcessID == pid) {
      HANDLE thread_handle =
          OpenThread(THREAD_SUSPEND_RESUME, FALSE, th32.th32ThreadID);
      if (thread_handle) {
        SuspendThread(thread_handle);
      }
      CloseHandle(thread_handle);
    }
  }
  CloseHandle(snapshot);
  return 0;
}

int delete_process(DWORD pid) {
  PROCESSENTRY32 process_entry_32 = {};
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  BOOL ret = TRUE;
  if (process_entry_32.dwSize != 0) {
    std::cout << "ERROR: default initialization failed.\n";
    return -1;
  }
  process_entry_32.dwSize = sizeof(PROCESSENTRY32);
  if (snapshot == INVALID_HANDLE_VALUE) {
    std::cout << "ERROR: CreateToolhelp32Snapshot() failed.\n";
    return -1;
  }
  ret = Process32First(snapshot, &process_entry_32);
  if (!ret) {
    std::cout << "ERROR: Process32First() failed.\n";
    return -1;
  }
  while (ret) {
    if (process_entry_32.th32ProcessID == pid) {
      HANDLE hProcess1 =
          OpenProcess(PROCESS_TERMINATE, FALSE, process_entry_32.th32ProcessID);
      TerminateProcess(hProcess1, 0);
      ret = FALSE;
      break;
    }
    ret = ret && Process32Next(snapshot, &process_entry_32);
    if (!ret) {
      std::cout << "INFO: pid NOT FOUND.\n";
    }
  }
  return 0;
}
#endif

int ConcretePIDInformation::get_pid_info() const {
  std::cout << "PID: \n";
#ifndef __linux__
  PROCESSENTRY32 process_entry_32;
  HANDLE handle_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  BOOL ret = TRUE;
  memset(&process_entry_32, 0, sizeof(PROCESSENTRY32));  // unsafe but have to
  process_entry_32.dwSize = sizeof(PROCESSENTRY32);
  if (handle_snapshot == INVALID_HANDLE_VALUE) {
    std::cout << "Warning! CreateToolhelp32Snapshot() failed.\n";
    return -1;
  }
  ret = Process32First(handle_snapshot, &process_entry_32);
  if (!ret) {
    std::cout << "Warning! Process32First() failed.\n";
    return -1;
  }
  std::vector<std::pair<std::string, int> > vec;
  std::pair<std::string, int> result;
  std::string ss;
  while (ret) {
    for (int i = 0; i < sizeof(process_entry_32.szExeFile) /
                            sizeof(process_entry_32.szExeFile[0]);
         i++) {
      ss += process_entry_32.szExeFile[i];
    }
    result.first = ss;
    result.second = process_entry_32.th32ProcessID;
    vec.push_back(result);
    ret = ret && Process32Next(handle_snapshot, &process_entry_32);
    ss = "";
  }
  int current_pid = (int)_getpid();
  std::cout << "    pid"
            << "          "
            << " exe_name\n";
  for (auto i : vec) {
    if (i.second == current_pid) {
      std::cout << "->  " << i.second << "          " << i.first << "\n";
    } else {
      std::cout << "    " << i.second << "          " << i.first << "\n";
    }
  }
#else
  const std::string PROC_DIRECTORY{"/proc/"};
  const std::string STAT_FILE_NAME{"/stat"};
  std::ifstream my_stream1(PROC_DIRECTORY + STAT_FILE_NAME);
  if (!my_stream1.is_open()) {
    std::cout << "Error! my_stream1() failed.\n";
    return -1;
  }
  std::string line1, key1;
  int total_proc = -1;
  bool flag1 = true;
  while (std::getline(my_stream1, line1) && flag1) {
    std::istringstream linestream1(line1);
    while (linestream1 >> key1 >> total_proc) {
      if (key1 == "processes") {
        flag1 = false;
        break;
      }
    }
  }
  my_stream1.close();
  int running_proc = -1;
  bool flag2 = true;
  std::string line2, key2;
  std::ifstream my_stream2(PROC_DIRECTORY + STAT_FILE_NAME);
  if (!my_stream2.is_open()) {
    std::cout << "Error! my_stream2() failed.\n";
    return -1;
  }
  while (std::getline(my_stream2, line2) && flag2) {
    std::istringstream linestream2(line2);
    while (linestream2 >> key2 >> running_proc) {
      if (key2 == "procs_running") {
        flag2 = false;
        break;
      }
    }
  }
  my_stream2.close();
  std::cout << "PID Info: \n";
  std::cout << "INFO: total processes are " << total_proc << "\n";
  std::cout << "INFO: running processes are " << running_proc << "\n";
  std::cout << "INFO: pid list below \n";
  if (system("ps -ao pid,comm | awk 'NR!=1 {print $0}'") !=
      0) {  // ok but use thread better
    std::cout << "INFO: read pid list failed.\n";
    return -1;
  }
#endif
  return 0;
}

int ConcreteSystemLogsInformation::get_system_logs_info() const {
  std::cout << "System Logs: \n";
#ifndef __linux__
  std::string folder_path = "C:\\Windows\\System32\\winevt\\Logs\\";
  std::string log_name = "System.evtx";
  std::vector<std::string> result =
      gsl::traverse_files_recursively(folder_path, log_name);
  if (result.empty()) {
    std::cout << "WRAN: no system logs.\n";
    return -1;
  }
  for (auto i : result) {
    gsl::show_file_content(i);
  }
#else
  std::string syslog_cmd = "cat /var/log/syslog";
  std::string syslog_output = gsl::exec_cmd(syslog_cmd);
  if (syslog_output.empty()) {
    std::cout << "WARN! syslog empty.\n";
    return -1;
  }
  // std::cout << syslog_output << "\n";  // syslog is too long. Uncomment it if
  // wanna show
  std::string startup_cmd = "last -x | less";
  std::string startup_output = gsl::exec_cmd(startup_cmd);
  if (startup_output.empty()) {
    std::cout << "WARN! startup log empty.\n";
    return -1;
  }
  std::cout << startup_output << "\n";
#endif
  return 0;
}

int ConcretePortsInformation::get_ports_info() const {
  std::cout << "Ports Info: \n";
#ifndef __linux__
  LPCSTR subkey = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Ports";
  LPCSTR item_name = "COM1:";
  /* Check if the registry exists */
  HKEY hkey;
  DWORD ret =
      RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCSTR)subkey, 0, KEY_READ, &hkey);
  if (ret != ERROR_SUCCESS) {
    std::cout << "ERROR: RegOpenKeyEx() failed.\n";
    return -1;
  }
  const int OFFSET_BYTES = 48;
  DWORD bytes_length = 48;
  DWORD data_length = bytes_length;
  PWCHAR buffer = (PWCHAR)malloc(sizeof(PWCHAR) * bytes_length);
  if (!buffer) {
    std::cout << "ERROR: malloc() failed.\n";
    return -1;
  }
  ret = RegQueryValueEx(hkey, (LPCSTR)item_name, nullptr, nullptr,
                        (BYTE*)buffer, &data_length);
  while (ret == ERROR_MORE_DATA) {
    bytes_length += OFFSET_BYTES;
    buffer = (PWCHAR)realloc(buffer, bytes_length);
    data_length = bytes_length;
    ret = RegQueryValueEx(hkey, (LPCSTR)item_name, nullptr, nullptr,
                          (BYTE*)buffer, &data_length);
  }
  if (ret != ERROR_SUCCESS) {
    std::cout << "ERROR: RegQueryValueEx() failed.\n";
    RegCloseKey(hkey);
    return -1;
  }
  RegCloseKey(hkey);
  std::cout << "INFO: COM1 " << (char*)buffer << "\n";
  free(buffer);
  buffer = nullptr;
#else
  std::string ports_output;
  std::string open_ports_cmd = "ss -l";
  ports_output = gsl::exec_cmd(open_ports_cmd);
  if (ports_output.empty()) {
    std::cout << "WARN: ports output empty.\n";
    return -1;
  }
  std::cout << ports_output;
#endif
  return 0;
}

int ConcreteInternetInformation::get_internet_info() const {
  std::cout << "Internet: \n";
#ifndef __linux__
  /* Declare and initialize variables */
  PIP_INTERFACE_INFO p_info = nullptr;
  ULONG buffer_out_length = 0;
  DWORD dw_ret;
  /* Make an initial call to get buffer_out_length variable */
  dw_ret = GetInterfaceInfo(nullptr, &buffer_out_length);
  if (dw_ret == ERROR_INSUFFICIENT_BUFFER) {
    p_info = (IP_INTERFACE_INFO*)malloc(sizeof(IP_INTERFACE_INFO) *
                                        buffer_out_length);
    if (!p_info) {
      std::cout << "ERROR: malloc() failed.\n";
      return -1;
    }
  }
  /* Make a second call to get actual data we need */
  dw_ret = GetInterfaceInfo(p_info, &buffer_out_length);
  int ret = (int)dw_ret;  // not safe but have to
  if (ret != 0) {
    std::cout << "ERROR: GetInterfaceInfo() failed.\n";
    return -1;
  }
  if (p_info) {
    std::cout << "INFO: number of adapters: " << p_info->NumAdapters << "\n";
    for (int i = 0; i < p_info->NumAdapters; i++) {
      std::cout << "INFO: adapter index[" << i
                << "]: " << p_info->Adapter[i].Index << "\n";
      std::cout << "INFO: adapter name[" << i
                << "]: " << p_info->Adapter[i].Name << "\n";
    }
  }
  free(p_info);
  p_info = nullptr;
#else
  std::string internet_connection_cmd = "nc -vz www.baidu.com 80";
  std::string internet_connection_output =
      gsl::exec_cmd(internet_connection_cmd);
  if (internet_connection_output.empty()) {
    std::cout << "WARN! internet connection check failed.\n";
    return -1;
  }
  std::cout << "INFO: " << internet_connection_output;
#endif
  std::cout << "INFO: internet connected\n";
  return 0;
}

int ConcreteProcessInformation::actions_on_process() const {
  std::cout << "Process: \n";
#ifndef __linux__
  int pid = user_input();
  if (pid < 0) {
    std::cout << "ERROR: user_input() failed.\n";
    return -1;
  }
  if (suspend_enabled_) {
    suspend_process(pid);
    std::cout << "INFO: suspend a process done.\n";
  } else {
    delete_process(pid);
    std::cout << "INFO: delete a process done.\n";
  }
#else
  std::cout << "INFO: PLEASE READ BELOW CAREFULLY\n";
  std::cout << "INFO: input 'pause, resume, or delete' to "
               "suspend/continue/kill a process.\n";
  bool flag = true;
  int ret1, ret2, ret3;
  std::string operation_mode, input_pid, cmd;
  while (flag) {
    std::cout << "INFO: input your choice: ";
    getline(std::cin, operation_mode);
    std::regex expression("\\s+");
    operation_mode = std::regex_replace(operation_mode, expression, " ");
    ret1 = operation_mode.compare("pause");
    ret2 = operation_mode.compare("resume");
    ret3 = operation_mode.compare("delete");
    if (ret1 == 0) {
      std::cout << "----- PAUSE enabled.\n";
      while (input_pid.empty()) {
        std::cout << "INFO: input PID that you want to PAUSE: ";
        getline(std::cin, input_pid);
        if (gsl::pid_range_check(input_pid) != 0) {
          std::cout << "INFO: pid NOT exist. try again.\n";
          input_pid = "";
        }
      }
      cmd = "kill -STOP " + input_pid;
      gsl::exec_cmd(cmd);
      std::cout << "INFO: " << input_pid << " is paused.\n";
      flag = false;
    } else if (ret2 == 0) {
      std::cout << "----- RESUME enabled.\n";
      while (input_pid.empty()) {
        std::cout << "INFO: input PID that you want to RESUME: ";
        getline(std::cin, input_pid);
        if (gsl::pid_range_check(input_pid) != 0) {
          std::cout << "INFO: pid NOT exist. try again.\n";
          input_pid = "";
        }
      }
      cmd = "kill -CONT " + input_pid;
      std::cout << "cmd: " << cmd << "\n";
      gsl::exec_cmd(cmd);
      std::cout << "INFO: " << input_pid << " is resumed.\n";
      flag = false;
    } else if (ret3 == 0) {
      std::cout << "----- DELETE enabled.\n";
      while (input_pid.empty()) {
        std::cout << "INFO: input PID that you want to DELETE: ";
        getline(std::cin, input_pid);
        if (gsl::pid_range_check(input_pid) != 0) {
          std::cout << "INFO: pid NOT exist. try again.\n";
          input_pid = "";
        }
      }
      cmd = "kill " + input_pid;
      gsl::exec_cmd(cmd);
      std::cout << "INFO: " << input_pid << " is deleted.\n";
      flag = false;
    } else {
      std::cout << "wrong input, try again.\n" << std::flush;
    }
  }
#endif
  return 0;
}

int ConcreteRegistryInformation::get_registry_info() const {
  std::cout << "Registry: \n";
#ifndef __linux__
  /* create key */
  LPCSTR subkey = "Environment";
  DWORD disposition;
  HKEY hkey;
  DWORD ret;
  ret = RegCreateKeyEx(HKEY_CURRENT_USER, (LPCSTR)subkey, 0, nullptr,
                       REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hkey,
                       &disposition);
  if (ret != ERROR_SUCCESS) {
    std::cout << "ERROR: RegCreateKeyEx() failed.\n";
    return -1;
  }
  RegCloseKey(hkey);
  /* read existed key message */
  LPCSTR item_name = "TEMP";  // attention: no PATH in some items
  HKEY hkey2;
  DWORD total_bytes_read = 1024;
  DWORD message_bytes_read = total_bytes_read;
  DWORD offset_bytes = 1024;
  PWCHAR buffer = (PWCHAR)malloc(sizeof(PWCHAR) * total_bytes_read);
  if (!buffer) {
    std::cout << "ERROR: malloc() failed.\n";
    return -1;
  }
  // check if the registry exists
  ret = RegOpenKeyEx(HKEY_CURRENT_USER, (LPCSTR)subkey, 0, KEY_READ, &hkey2);
  if (ret != ERROR_SUCCESS) {
    std::cout << "Error! RegOpenKeyEx() failed.\n";
    return -1;
  }
  ret = RegQueryValueEx(hkey2, (LPCSTR)item_name, nullptr, nullptr,
                        (BYTE*)buffer, &message_bytes_read);
  while (ret == ERROR_MORE_DATA) {
    // get a buffer that is big enough.
    total_bytes_read += offset_bytes;
    buffer = (PWCHAR)realloc(buffer, total_bytes_read);
    message_bytes_read = total_bytes_read;
    ret = RegQueryValueEx(hkey2, (LPCSTR)item_name, nullptr, nullptr,
                          (BYTE*)buffer, &message_bytes_read);
  }
  if (ret != ERROR_SUCCESS) {
    RegCloseKey(hkey2);
    return -1;
  }
  RegCloseKey(hkey2);
  std::cout << "INFO: key value is " << (char*)buffer << "\n";
  free(buffer);
  buffer = nullptr;
#else
  std::cout << "WARN: Linux has no registry except configuration files from "
               "third-party softwares.\n";
#endif
  return 0;
}

int ConcreteFirewallInformation::get_firewall_info() const {
  std::cout << "Firewall: \n";
  char* file = nullptr;
#ifndef __linux__
  std::string folder_path = "C:\\Windows\\System32\\LogFiles\\FireWall\\";
  std::vector<std::string> vec = firewall_logs_list(folder_path);
  if (vec.empty()) {
    std::cout
        << "WARN: firewall log list empty.\n";  // check: if no firewell logs
    return -1;
  }
  int ret = -1;
  const int EXTRA_BYTES = 500;
  for (int i = 0; i < vec.size(); i++) {
    ret = strcmp(vec[i].c_str(), "pfirewall.log");
    if (ret == 0) {
      size_t string_length = (folder_path + vec[i]).length();
      file = (char*)malloc(sizeof(char) * string_length + EXTRA_BYTES);
      strcpy_s(file, strlen(folder_path.c_str()) + 1, folder_path.c_str());
      strcat_s(file, sizeof(file) + EXTRA_BYTES, vec[i].c_str());
      break;
    }
  }
  if (!file) {
    std::cout << "WARN: pfirewall.log NOT FOUND.\n";
    return -1;
  }
  std::cout << "INFO: pfirewall.log FOUND.\n";
  std::fstream my_stream;
  my_stream.open(file, std::ios::in);
  if (!my_stream.is_open()) {
    std::cout << "ERROR: std::fstream::open() failed.\n";
    free(file);  // dont forget
    file = nullptr;
    return -1;
  }
  std::string ss;
  while (getline(my_stream, ss)) {
    std::cout << ss << "\n";
  }
  my_stream.close();
  free(file);  // dont forget
  file = nullptr;
#else
  std::cout << "WARN: Linux has no firewall logs.\n";
#endif
  return 0;
}

int ConcreteLogonStatusInformation::get_logon_status_info() const {
  std::cout << "Logon Status: \n";
#ifndef __linux__
  std::string folder_path = "C:\\Windows\\System32\\winevt\\Logs\\";
  std::string log_file = "Security.evtx";
  std::vector<std::string> result =
      gsl::traverse_files_recursively(folder_path, log_file);
  if (result.empty()) {
    std::cout << "WRAN: no Logon logs.\n";
    return -1;
  }
  for (auto i : result) {
    gsl::show_file_content(i);
  }

#else
  std::cout << "see SYSTEM LOG INFO above.\n";
#endif
  return 0;
}