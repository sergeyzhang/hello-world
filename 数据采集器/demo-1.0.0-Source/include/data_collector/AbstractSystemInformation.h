#ifndef ABSTRACT_SYSTEM_INFORMATION_H
#define ABSTRACT_SYSTEM_INFORMATION_H

#include "AbstractDeviceInformation.h"

class AbstractSystemInformation {
 public:
  virtual CPU* create_cpu_information() const = 0;
  virtual Memory* create_memory_information() const = 0;
  virtual Disk* create_disk_information() const = 0;
  virtual Account* create_account_information() const = 0;
  virtual PID* create_pid_information() const = 0;
  virtual SystemLogs* create_system_logs_information() const = 0;
  virtual Ports* create_ports_information() const = 0;
  virtual Internet* create_internet_information() const = 0;
  virtual Process* create_process_information() const = 0;
  virtual Registry* create_registry_information() const = 0;
  virtual Firewall* create_firewall_information() const = 0;
  virtual LogonStatus* create_logon_status_information()
      const = 0;  // security ID 4624,4647,4634
};
#endif