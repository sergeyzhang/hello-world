#ifndef CONCRETE_ACTIVITY_RECOGNTION_H
#define CONCRETE_ACTIVITY_RECOGNTION_H

#include "AbstractActivityRecognition.h"
#include "ConcreteActivities.h"

class ConcreteActivityRecognition : public AbstractActivityRecognition {
 public:
  Activity1* create_activity1_information() const override;
  Activity2* create_activity2_information() const override;
  Activity3* create_activity3_information() const override;
};

#endif

