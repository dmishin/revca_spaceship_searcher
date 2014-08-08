#ifndef __SINGLEROT_HPP_INCLUDED__
#define __SINGLEROT_HPP_INCLUDED__

#include "streamed_analysys.hpp"

class SinglerotCoralFilter: public PatternFilter{
public:
  virtual bool check( const Pattern &p);
};

class MargolusBinaryRule;
extern MargolusBinaryRule singlerot;

#endif
