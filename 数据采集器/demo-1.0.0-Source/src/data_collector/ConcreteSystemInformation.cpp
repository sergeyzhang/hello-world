#include "../../include/data_collector/ConcreteSystemInformation.h"

CPU* ConcreteSystemInformation::create_cpu_information() const {
  return new ConcreteCpuInformation();
}

Memory* ConcreteSystemInformation::create_memory_information() const {
  return new ConcreteMemoryInformation();
}

Disk* ConcreteSystemInformation::create_disk_information() const {
  return new ConcreteDiskInformation();
}

Account* ConcreteSystemInformation::create_account_information() const {
  return new ConcreteAccountInformation();
}

PID* ConcreteSystemInformation::create_pid_information() const {
  return new ConcretePIDInformation();
}

SystemLogs* ConcreteSystemInformation::create_system_logs_information() const {
  return new ConcreteSystemLogsInformation();
}

Ports* ConcreteSystemInformation::create_ports_information() const {
  return new ConcretePortsInformation();
}

Internet* ConcreteSystemInformation::create_internet_information() const {
  return new ConcreteInternetInformation();
}

Process* ConcreteSystemInformation::create_process_information() const {
  return new ConcreteProcessInformation();
}

Registry* ConcreteSystemInformation::create_registry_information() const {
  return new ConcreteRegistryInformation();
}

Firewall* ConcreteSystemInformation::create_firewall_information() const {
  return new ConcreteFirewallInformation();
}

LogonStatus* ConcreteSystemInformation::create_logon_status_information()
    const {
  return new ConcreteLogonStatusInformation();
}