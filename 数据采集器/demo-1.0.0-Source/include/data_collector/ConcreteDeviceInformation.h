#ifndef CONCRETE_DEVICE_INFORMATION_H
#define CONCRETE_DEVICE_INFORMATION_H

#include <assert.h>

#include <cmath>
#include <regex>

#include "../../include/utils/Help.h"
#include "AbstractDeviceInformation.h"
#include "CpuInfo.h"

class ConcreteCpuInformation : public CPU {
 public:
  int get_cpu_use_rate() const override;
};

class ConcreteMemoryInformation : public Memory {
 public:
  int get_memory_volume() const override;
};

class ConcreteDiskInformation : public Disk {
 public:
  int get_disk_info() const override;
};

class ConcreteAccountInformation : public Account {
 public:
  int get_account_info() const override;

 private:
  int add_account() const;
  int delete_account() const;
  int exec_concat_cmd(const std::string& src) const;
};

class ConcretePIDInformation : public PID {
 public:
  int get_pid_info() const override;
};

class ConcreteSystemLogsInformation : public SystemLogs {
 public:
  int get_system_logs_info() const override;
};

class ConcretePortsInformation : public Ports {
 public:
  int get_ports_info() const override;
};

class ConcreteInternetInformation : public Internet {
 public:
  int get_internet_info() const override;
};

static bool suspend_enabled_ = false;
class ConcreteProcessInformation : public Process {
 public:
  int actions_on_process() const override;
};

class ConcreteRegistryInformation : public Registry {
 public:
  int get_registry_info() const override;
};

class ConcreteFirewallInformation : public Firewall {
 public:
  int get_firewall_info() const override;

 private:
  std::vector<std::string> firewall_logs_list(
      const std::string& folder_path) const;
};

class ConcreteLogonStatusInformation : public LogonStatus {
 public:
  int get_logon_status_info() const override;
};

#endif  // end of CONCRETE_DEVICE_INFORMATION
