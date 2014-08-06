#ifndef __BRUTEFORCE_PATTERN_SOURCE_INCLUDED__
#define __BRUTEFORCE_PATTERN_SOURCE_INCLUDED__

#include "streamed_analysys.hpp"
#include "pattern_enumerator.hpp"

class BruteforceSource: public AbstractPatternSource{
  pattern_enumerator patterns;
public:
  BruteforceSource( int pattern_size ): patterns(pattern_size){};
  virtual bool get( Pattern & p, int& g );
  virtual bool is_closed();
};



#endif
