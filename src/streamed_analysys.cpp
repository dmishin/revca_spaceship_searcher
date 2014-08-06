#include "streamed_analysys.hpp"
#include <iostream>
using namespace std;

AbstractPatternSource::AbstractPatternSource()
  :processed(0)
{
}


size_t AbstractPatternSource::get_processed()
{
  std::unique_lock<std::mutex> _lock_stream(lock);
  return processed;
}

size_t Library::get_size()const
{
  std::unique_lock<std::mutex> _locker(lock);
  return catalog.size();
}


void Library::put( const AnalysysResult & result, const Pattern &bestPattern )
{
  std::unique_lock<std::mutex> _locker(lock);
  auto iitem = catalog.find( bestPattern );
  if (iitem != catalog.end() ){
    //already have it
    iitem->second.count ++;
  }else{
    LibraryRecord rcd;
    rcd.count = 1;
    rcd.period = result.period;
    rcd.offset = result.offset;
    catalog[bestPattern] = rcd;
  }
}
void Library::dump( std::ostream &os )
{
  std::unique_lock<std::mutex> _locker(lock);
  /** new format
  os<<"{\"version\":1,"<<endl
    <<"\"size\":"<<catalog.size()<<','
    <<"\"catalog\":["<<endl;
  bool first = true;
  for( auto item : catalog ){
    if (first) first=false; else os <<",";
    os << "{ \"pattern\": "<<'"'<<item.first.to_rle()<<'"'<<",";
    os << " \"count\":"<<item.second.count<<",";
    os << " \"period\":"<<item.second.period<<",";
    os << " \"offset\":"<<item.second.offset<<"}"<<endl;
  }
  os << "]}"<<endl;
  */
  os << '[' << endl;
  bool first = true;
  for( auto &item : catalog ){
    const Pattern & p(item.first);
    const LibraryRecord &rec(item.second);

    if (first)
      first = false;
    else
      os << ',';

    os <<'{'
       << "\"result\":{"
       << "\"analysed_generations\":"<<10000<<','
       << "\"dx\":" << rec.offset[0]<<','
       << "\"dy\":" << rec.offset[1]<<','
       << "\"period\":" << rec.period<<','
       << "\"cells\":" << p
       << "},"
       << "\"count\":"<<rec.count<<','
       << "\"key\":" << '"' << p.to_rle() << '"'
       << '}' 
       << endl;
  }
  os << ']' << endl;
}

void Library::read( std::istream &is )
{
  std::unique_lock<std::mutex> _locker(lock);
  //TODO
}

