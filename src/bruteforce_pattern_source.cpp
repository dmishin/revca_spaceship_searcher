#include "bruteforce_pattern_source.hpp"

bool BruteforceSource::get( Pattern & p, int& g )
{
  patterns.for_pattern( [&p](int idx, int x, int y){
      p.append(x,y);
    } );
  patterns.next();
  processed ++;
  return true;
}
bool BruteforceSource::is_closed()
{
  return false;
}

