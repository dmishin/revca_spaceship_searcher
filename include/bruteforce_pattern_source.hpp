#ifndef __BRUTEFORCE_PATTERN_SOURCE_INCLUDED__
#define __BRUTEFORCE_PATTERN_SOURCE_INCLUDED__

#include "streamed_analysys.hpp"
#include "pattern_enumerator.hpp"

class BruteforceSource: public AbstractPatternSource{
  pattern_enumerator patterns;
protected:
  virtual bool get_nofilter( Pattern & p, int& g );
public:
  BruteforceSource( int pattern_size ): patterns(pattern_size){};
  BruteforceSource( const std::vector<int> &index): patterns(index){};
  virtual bool is_closed();
  virtual std::string get_position_text()const;
};



#endif
