#include <sstream>
#include "bruteforce_pattern_source.hpp"

bool BruteforceSource::get_nofilter( Pattern & p, int& g )
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


std::string BruteforceSource::get_position_text()const
{
  std::unique_lock<std::mutex> _lock_stream(lock);
  std::stringstream ss;
  ss<<"index: ";
  bool first=true;
  for( int ci: patterns.state ){
    if (first) first = false;
    else ss<<',';
    ss<<ci;
  }
  return ss.str();
}
