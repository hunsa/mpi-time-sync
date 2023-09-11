
#ifndef MPITS_CLOCKOFFSET_CLASS_H_
#define MPITS_CLOCKOFFSET_CLASS_H_

class ClockOffset {

private:
  double timestamp;
  double offset;

public:
  ClockOffset(double timestamp = 0, double offset = 0);
  ~ClockOffset();

  double get_timestamp(void);
  double get_offset(void);
};


#endif /*  MPITS_CLOCKOFFSET_CLASS_H_  */

