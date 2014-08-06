#ifndef __STREAMED_ANALYSYS_INCLUDED__
#define __STREAMED_ANALYSYS_INCLUDED__
#include "rule.hpp"
#include "pattern.hpp"
#include "field.hpp"
#include "analyze.hpp"
/*

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <set>
#include <fstream>
#include <cstring>
#include <ctime>
#include <unordered_map>

#include <thread>
//#include <condition_variable>
#include <deque>
#include <memory>
*/
#include <mutex>
class Library;

class AbstractPatternSource{
protected:
  std::mutex lock;
  size_t processed;
public:
  AbstractPatternSource();
  size_t get_processed();
  virtual bool is_closed()=0;
  virtual bool get( Pattern & p, int& g )=0;
};

class Library{
public:
  
  struct LibraryRecord{
    int count;
    int period;
    Cell offset;
  };
  typedef std::unordered_map<Pattern, LibraryRecord> catalog_t;
  mutable std::mutex lock;
private:
  catalog_t catalog;
public:
  void put( const AnalysysResult & result, const Pattern &bestPattern );
  void dump( std::ostream &os );
  void read( std::istream &is );
  size_t get_size()const;
};


#endif
