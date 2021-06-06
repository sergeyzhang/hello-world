#include "../../include/data_collector/ConcreteActivities.h"

int ConcreteActivity1::get_activity1_data() const {
  std::cout << "Activity1 do something\n";
  return 0;
}

int ConcreteActivity2::get_activity2_data() const {
  std::cout << "Activity2 do something\n";
  return 0;
}

int ConcreteActivity3::get_activity3_data() const {
  std::cout << "Activity3 do something\n";
  return 0;
}
