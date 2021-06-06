#ifndef CONCRETE_ACTIVITIES_H
#define CONCRETE_ACTIVITIES_H

#include <iostream>

#include "AbstractActivityRecognition.h"

class ConcreteActivity1 : public Activity1 {
 public:
  int get_activity1_data() const override;
};

class ConcreteActivity2 : public Activity2 {
 public:
  int get_activity2_data() const override;
};

class ConcreteActivity3 : public Activity3 {
 public:
  int get_activity3_data() const override;
};

#endif
