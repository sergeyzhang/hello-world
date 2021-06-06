#ifndef ABSTRACT_DEVICE_INFORMATION_H
#define ABSTRACT_DEVICE_INFORMATION_H

#include "../utils/Help.h"

class CPU {
 public:
  virtual ~CPU(){};
  virtual int get_cpu_use_rate() const = 0;
};

class Memory {
 public:
  virtual ~Memory(){};
  virtual int get_memory_volume() const = 0;
};

class Disk {
 public:
  virtual ~Disk(){};
  virtual int get_disk_info() const = 0;
};

class Account {
 public:
  virtual ~Account(){};
  virtual int get_account_info() const = 0;
};

class PID {
 public:
  virtual ~PID(){};
  virtual int get_pid_info() const = 0;
};

class SystemLogs {
 public:
  virtual ~SystemLogs(){};
  virtual int get_system_logs_info() const = 0;
};

class Ports {
 public:
  virtual ~Ports(){};
  virtual int get_ports_info() const = 0;
};

class Internet {
 public:
  virtual ~Internet(){};
  virtual int get_internet_info() const = 0;
};

class Process {
 public:
  virtual ~Process(){};
  virtual int actions_on_process() const = 0;
};

class Registry {
 public:
  virtual ~Registry(){};
  virtual int get_registry_info() const = 0;
};

class Firewall {
 public:
  virtual ~Firewall(){};
  virtual int get_firewall_info() const = 0;
};

class LogonStatus {
 public:
  virtual ~LogonStatus(){};
  virtual int get_logon_status_info() const = 0;
};

#endif  // end of ABSTRACT_DEVICE_INFORMATION_H