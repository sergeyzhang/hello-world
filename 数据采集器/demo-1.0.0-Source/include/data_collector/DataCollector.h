#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <iostream>
#include <assert.h>
#include "ConcreteSystemInformation.h"
#include "ConcreteActivityRecognition.h"

class DataCollector {
 public:
  DataCollector();
  ~DataCollector();
  int run();

 private:
  ConcreteSystemInformation* system_information_;
  ConcreteActivityRecognition* activity_recognition_;
};

#endif
