#include "../../include/data_collector/ConcreteActivityRecognition.h"

Activity1* ConcreteActivityRecognition::create_activity1_information() const {
  return new ConcreteActivity1();
}
Activity2* ConcreteActivityRecognition::create_activity2_information() const {
  return new ConcreteActivity2();
}
Activity3* ConcreteActivityRecognition::create_activity3_information() const {
  return new ConcreteActivity3();
}
