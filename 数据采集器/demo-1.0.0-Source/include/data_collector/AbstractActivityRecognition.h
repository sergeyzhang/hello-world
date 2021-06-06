#ifndef ABSTRACT_ACTIVITY_RECOGNITION_H
#define ABSTRACT_ACTIVITY_RECOGNITION_H

#include "AbstractActivities.h"

class AbstractActivityRecognition {
 public:
  virtual Activity1* create_activity1_information() const = 0;
  virtual Activity2* create_activity2_information() const = 0;
  virtual Activity3* create_activity3_information() const = 0;
};

#endif
