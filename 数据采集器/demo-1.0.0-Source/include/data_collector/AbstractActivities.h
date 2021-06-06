#ifndef ABSTRACT_ACTIVITIES_H
#define ABSTRACT_ACTIVITIES_H

class Activity1 {
 public:
  virtual ~Activity1(){};
  virtual int get_activity1_data(void) const = 0;
};

class Activity2 {
 public:
  virtual ~Activity2(){};
  virtual int get_activity2_data(void) const = 0;
};

class Activity3 {
 public:
  virtual ~Activity3(){};
  virtual int get_activity3_data(void) const = 0;
};

#endif
