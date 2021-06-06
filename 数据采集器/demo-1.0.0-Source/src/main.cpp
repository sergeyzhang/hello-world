#include "../include/utils/Client.h"

int main() {
  Client client;
  client.run_data_collector();
  client.run_data_transmitter();
  getchar();
  return 0;
}