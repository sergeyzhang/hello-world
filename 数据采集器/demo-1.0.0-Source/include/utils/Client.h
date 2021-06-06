#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>

class Client {
 public:
  int run_data_collector();
  int run_data_transmitter();

 private:
  bool data_collection_succeed_ = false;
};

#endif


