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
*/
#include <memory>
#include <mutex>
class Library;

class PatternFilter;

class AbstractPatternSource{
private:
  std::vector< std::unique_ptr< PatternFilter > > filters;
  bool check_pattern( const Pattern &p)const;
protected:
  mutable std::mutex lock;
  size_t processed;
  virtual bool get_nofilter( Pattern & p, int& g )=0;
  bool closed;
public:
  AbstractPatternSource();
  bool get( Pattern & p, int& g );
  size_t get_processed();
  bool is_closed();
  void add_filter( std::unique_ptr<PatternFilter> filter );
  virtual std::string get_position_text()const;
};

class Library{
public:
  
  struct LibraryRecord{
    int count;
    int period;
    Cell offset;
    int max_generations;
  };
  typedef std::unordered_map<Pattern, LibraryRecord> catalog_t;
  mutable std::mutex lock;
private:
  catalog_t catalog;
public:
  bool store_hit_count;
  Library():store_hit_count(true){};
  void put( const AnalysysResult & result, const Pattern &bestPattern );
  void dump( std::ostream &os );
  void read( std::istream &is );
  size_t get_size()const;
};

class PatternFilter{
public:
  virtual bool check( const Pattern &p )=0;
};
class NoFilter: public PatternFilter{
public:
  virtual bool check( const Pattern & ){return true;};
};
#endif
