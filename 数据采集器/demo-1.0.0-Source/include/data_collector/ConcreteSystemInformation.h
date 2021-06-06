#ifndef CONCRETE_SYSTEM_INFORMATION_H
#define CONCRETE_SYSTEM_INFORMATION_H

#include "AbstractSystemInformation.h"
#include "ConcreteDeviceInformation.h"

class ConcreteSystemInformation : public AbstractSystemInformation {
 public:
  CPU* create_cpu_information() const override;
  Memory* create_memory_information() const override;
  Disk* create_disk_information() const override;
  Account* create_account_information() const override;
  PID* create_pid_information() const override;
  SystemLogs* create_system_logs_information() const override;
  Ports* create_ports_information() const override;
  Internet* create_internet_information() const override;
  Process* create_process_information() const override;
  Registry* create_registry_information() const override;
  Firewall* create_firewall_information() const override;
  LogonStatus* create_logon_status_information() const override;
};

#endif