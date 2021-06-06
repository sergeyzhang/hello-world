#ifndef HELP_H
#define HELP_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
/* Keep Sequence Format */
#include <stdio.h>     //1
#include <stdlib.h>    //2
#include <windows.h>   //3
#include <winsock2.h>  //4
#include <ws2tcpip.h>  //5
#include <lm.h>        //6
#include <sddl.h>      //7
#include <conio.h>     //8
#include <process.h>   //10
#include <TlHelp32.h>  //11
#include <Windows.h>   //12
#include <initguid.h>  //13
#include <Setupapi.h>  //14
#include <ws2ipdef.h>  //15
#include <iphlpapi.h>  //16
#include <vector>      //17
#include <locale>      //18
#include <codecvt>     //19
#include <string>      //20
#include <iostream>    //21

#pragma comment(lib, "ws2_32.lib")    // 1
#pragma comment(lib, "mswsock.lib")   // 2
#pragma comment(lib, "advapi32.lib")  // 3
#pragma comment(lib, "netapi32.lib")  // 4
#pragma comment(lib, "iphlpapi.lib")  // 5

#else

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#endif

namespace gsl {  // do: guidline support library

#ifndef __linux__
static std::wstring string_to_wstring(const std::string& src) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  // std::string narrow = converter.to_bytes(wide_utf16_source_string);
  std::wstring result;
  result = converter.from_bytes(src);
  return result;
}

static std::string wstring_to_string(const std::wstring& src) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::string result;
  result = converter.to_bytes(src);
  return result;
}

static LPCWSTR string_to_LPCWSTR(const std::string& src) {
  std::wstring ws;
  ws = string_to_wstring(src);
  LPCWSTR wide_string;
  wide_string = ws.c_str();
  return wide_string;
}
#endif

static int ensures(char* buffer, int data_length) {
  int sum = 0;
  for (int i = 0; i < data_length; ++i) {
    sum |= buffer[i];
    if (sum != 0) {
      break;
      return -1;
    }
  }
  return 0;
}

static std::string exec_cmd(std::string cmd) {  // dont const string& or string&
  cmd.append(" 2>&1");
  std::string cmd_output;
#ifdef __linux__
  FILE* stream;
  stream = popen(cmd.c_str(), "r");
  if (!stream) {
    return "";
  }
  const int BUFLEN = 1024;
  char buffer[BUFLEN];
  while (!feof(stream)) {
    if (fgets(buffer, BUFLEN, stream) != nullptr) {
      cmd_output.append(buffer);
    }
  }
  pclose(stream);
#endif
  return cmd_output;
}

static int pid_range_check(const std::string& pid) {
  struct stat stats;
  std::string ss = "/proc/" + pid;
  if (stat(ss.c_str(), &stats) == -1) {
    return -1;
  }
  return 0;
}

static std::vector<std::string> traverse_files_recursively(
    const std::string& folder_path, const std::string& log_file) {
  /*
   * folder_path exmple:
   *
   * std::string folder_path = "C:\\Windows\\System32\\winevt\\Logs\\";
   *
   */
  std::string file_name = folder_path + "*.*";
  std::vector<std::string> vec;
  int ret = -1;
  std::string ss;
#ifndef __linux__
  WIN32_FIND_DATA find_data;
  HANDLE hfind = FindFirstFileA(file_name.c_str(),
                                &find_data);  // wstring to LPCWSTR
  if (hfind == INVALID_HANDLE_VALUE) {
    std::cout << "WARN: FindFirstFile() failed.\n";
    FindClose(hfind);
    return {};
  }
  while (FindNextFile(hfind, &find_data)) {
    if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      if (log_file.compare("System.evtx") == 0) {
        std::cout << "INFO: " << find_data.cFileName << " \n";
      }
      ss = find_data.cFileName;  // ignore compiler warnings
      ret = ss.compare(log_file);
      if (ret == 0) {
        vec.push_back(ss);
      }
    }
  }
  FindClose(hfind);
#endif
  return vec;
}

static int show_file_content(const std::string& file_name) {
  /*
   * file_name exmple:
   *
   * std::string file_name =
   * "C:\\Windows\\System32\\winevt\\Logs\\Security.evtx";
   *
   */
  const int RESULT_SIZE = 5120;  // waste but better than unknown
  const int BUFFER_SIZE = 1024;
  char result[RESULT_SIZE];
  char buffer[BUFFER_SIZE];
  std::fill(result, result + sizeof(result), 0);
  std::fill(buffer, buffer + sizeof(buffer), 0);
  gsl::ensures(result, sizeof(result));
  gsl::ensures(buffer, sizeof(buffer));

  std::string log_name = file_name.substr(0, file_name.rfind("."));
  const int LEN1 = (int)log_name.size();
  char* tmp1 = (char*)malloc(sizeof(char) * (LEN1 + 1));
  if (!tmp1) {
    return -1;
  }
#ifndef __linux__
  strcpy_s(tmp1, LEN1 + 1, log_name.c_str());
  std::string pre_cmd, post_cmd;
  pre_cmd = "wevtutil qe ";
  post_cmd = " /c:1 /rd:true /f:text";  // 1 rows only
  const int LEN2 = (int)pre_cmd.size() + LEN1;
  const int LEN3 = (int)post_cmd.size();
  char* cmd = (char*)malloc(sizeof(char) * (LEN2 + LEN3 + 1));
  if (!cmd) {
    return -1;
  }
  strcpy_s(cmd, (int)pre_cmd.size() + 1, pre_cmd.c_str());
  strcat_s(cmd, (LEN2 + LEN3 + 1), tmp1);
  strcat_s(cmd, (LEN2 + LEN3 + 1), post_cmd.c_str());
  FILE* pipe = _popen(cmd, "r");
  if (!pipe) {
    return -1;
  }
  while (!feof(pipe)) {
    if (fgets(buffer, 1024, pipe)) {
      strcat_s(result, 5120, buffer);
    }
  }
  free(tmp1);
  tmp1 = nullptr;
  free(cmd);
  cmd = nullptr;
  _pclose(pipe);
#endif
  std::cout << result << "\n";
  return 0;
}

}  // namespace gsl
#endif