#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include "../../include/data_collector/FileWatcher.h"

std::vector<std::string> get_all_file_names_within_folder(std::string folder) {
  std::vector<std::string> names;
  std::string search_path = folder + "/*.*";
  WIN32_FIND_DATA find_data;
  HANDLE hfind = FindFirstFile(search_path.c_str(), &find_data);
  while (FindNextFile(hfind, &find_data)) {
    if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      names.push_back(find_data.cFileName);
    }
  }
  if (hfind != INVALID_HANDLE_VALUE) {
    FindClose(hfind);
  }
  return names;
}

int check_disk_existence(const std::string &disk) {
  DWORD ftyp = GetFileAttributesA(disk.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES) {
    std::cout << "Invalid disk path.\n";
    return -1;
  }
  if (ftyp & FILE_ATTRIBUTE_DIRECTORY) {
    std::cout << "Disk path found.\n";
    return 0;  
  }
  std::cout << "Invalid disk path.\n";
  return -1;  
}

int main() {
  // 1 ������һ��Ŀ¼+��Ƶ
#if (0)
  std::string disk = "C:\\Windows\\System32\\LogFiles\\FireWall";
  for (const auto &entry :
       std::experimental::filesystem::directory_iterator(disk)) {
    std::cout << entry.path() << std::endl;
  }
#endif


  // 2 ���������е��ļ������ۺ�׺��ʲô
  std::cout << "=================================================\n";
  std::cout << "=================  File  Watcher  ===============\n";
  std::cout << "=================================================\n";
  std::cout << "* ********************************************* *\n";
  std::cout << "* You need to input disk or folder path, e.g.    \n";
  std::cout << "* [1] disk: C:\\ or C:     \n";
  std::cout << "* [2] folder: E:\\demo\\  \n";
  std::cout << "* ********************************************* *\n";
  std::string ss;
  bool flag1 = true;
  while (flag1) {
    std::cout << "User Input: ";
    getline(std::cin, ss);
    std::regex expression("\\s+");
    ss = std::regex_replace(ss, expression, " ");  // remove whitespace
    int ret = check_disk_existence(ss);
    if (ret < 0){
      std::cout << "Warning! Wrong input. Do it again.\n" << std::flush;
	}else{
      std::cout << "Your input is: " << ss << "\n";
      flag1 = false;
      break;
	}
  }
  std::vector<std::string> v = get_all_file_names_within_folder(ss);
  if (v.empty()) {
    std::cout << "Empty folder or disk!\n";
    getchar();
    return -1;
  }
  for (auto i : v) {
    std::cout << i << std::endl;
  }

  // 3 �����ڵݹ�Ŀ¼+��Ƶ
#if (0)
  for (const auto &entry :
       std::experimental::filesystem::recursive_directory_iterator(disk)) {
    std::cout << entry.path() << std::endl;
  }
#endif

  //�ļ�����ɾ�Ķ�
  FileWatcher fw{ss, std::chrono::milliseconds(1000)};
  fw.start([](std::string path_to_watch, FileStatus status) -> void {
    // Process only regular files, all other file types are ignored
    if (!std::experimental::filesystem::is_regular_file(
            std::experimental::filesystem::path(path_to_watch)) &&
        status != FileStatus::erased) {
      return;
    }

    std::fstream newfile;
    switch (status) {
      case FileStatus::created:
        std::cout << "File created: " << path_to_watch << '\n';

        break;
      case FileStatus::modified:
        std::cout << "File modified: " << path_to_watch << '\n';
        newfile.open(path_to_watch, std::ios::in);
        if (newfile.is_open()) {  // checking whether the file is open
          std::string tp;
          while (getline(newfile, tp)) {
            std::cout << tp << "\n";
          }
          newfile.close();  // close the file object.
        }
        break;
      case FileStatus::erased:
        std::cout << "File erased: " << path_to_watch << '\n';
        break;
      default:
        std::cout << "Error! Unknown file status.\n";
    }
  });
  return 0;
}