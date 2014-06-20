//#include "gason.hpp"
#include "picojson.h"

#include "rule.hpp"
#include "pattern.hpp"
#include "field.hpp"
#include "analyze.hpp"

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <set>
#include <fstream>
#include <cstring>
#include <ctime>
#include <unordered_map>

#include <thread>
#include <mutex>
//#include <condition_variable>
#include <deque>
#include <memory>

using namespace std;

class Library;
void parse_record( picojson::value &record, int & generation, Pattern &pattern );
void analyze_record( Analyzer &analyzer, int generation, const Pattern &pattern, Library &library );

std::mutex stdio_mtx;

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

size_t Library::get_size()const
{
  std::unique_lock<std::mutex> _locker(lock);
  return catalog.size();
}


class PatternSource{
  std::istream &stream;
  std::mutex lock;
  static const size_t buf_size=2048;
  char line_buffer[buf_size];
  size_t processed;
  bool closed;
public:
  PatternSource( std::istream &s ): stream(s), processed(0), closed(false){};
  bool get( Pattern & p, int& g );
  size_t get_processed();
  bool is_closed();
};

size_t PatternSource::get_processed()
{
  std::unique_lock<std::mutex> _lock_stream(lock);
  return processed;
}
bool PatternSource::is_closed()
{
  std::unique_lock<std::mutex> _lock_stream(lock);
  return closed;
}
bool PatternSource::get( Pattern & p, int &generation )
{
  picojson::value record;
  {
    std::unique_lock<std::mutex> _lock_stream(lock);
    if (closed) return false;
    stream.getline(line_buffer, buf_size);
    if (line_buffer[0]=='\0') {
      closed = true;
      return false;
    }
    //parsing 
    istringstream iss(line_buffer);
    iss >> record;
    processed ++;
    std::string err = picojson::get_last_error();
    if (! err.empty())
      throw std::logic_error(err);
  }
  //got record.
  //now sync is not needed

  parse_record( record, generation, p );
  return true;
}

void analysys_worker( Analyzer &analyzer, Library & lib, PatternSource& source )
{
  Pattern p;
  int generation;
  while(true){
    try{
      if (! source.get(p, generation) )
	break;
      analyze_record( analyzer, generation, p, lib);
    }catch(std::exception &err){
      unique_lock<mutex> _lock(stdio_mtx);
      cerr<<"#Error parsing line: ["<<err.what()<<"]"<<endl;
    }
    p.clear();
  }
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


void parse_record( picojson::value &record, int & generation, Pattern &pattern )
{
  if ( !record.is<picojson::object>() ) throw logic_error("record is not object");

  auto & record_g(record.get("g"));
  if (!record_g.is<double>()) throw logic_error("'g' must be number");
  generation = (int)record_g.get<double>();

  auto &record_p(record.get("p"));
  if (!record_p.is<picojson::array>()) throw logic_error("p is not array");
  
  picojson::array &cells_array = record_p.get<picojson::array>();
  size_t ncells = cells_array.size();
  for(size_t i=0; i!=ncells; ++i){
    auto & cells_array_i( cells_array[i] );
    if (! cells_array_i.is<picojson::array>() ) throw logic_error("pattern elements must be arrays");    
    picojson::array &xy = cells_array[i].get<picojson::array>();
    if ( xy.size() !=  2) throw logic_error("must have 2 coords");
    auto & x(xy[0]);
    auto & y(xy[1]);
    if ( ! (x.is<double>() && y.is<double>()) ) throw logic_error("x and y must be numbers");
    pattern.append( (int)xy[0].get<double>(),
		    (int)xy[1].get<double>() );
  }
}

int max_pattern_size=0;
void analyze_record( Analyzer &analyzer, int generation, const Pattern &pattern, Library &library )
{
  auto result = analyzer.process(pattern);
  
  if (result.resolution == AnalysysResult::CYCLE_FOUND &&
      result.offset != Cell(0,0)){

    //find the compact representation
    Pattern bestPattern = most_compact_form( pattern, result.period, analyzer.get_rule());    

    //normalizing rotation of the spaceship
    const Transform &tfm = normalizing_rotation( result.offset );
    result.offset = tfm(result.offset);
    bestPattern.transform(tfm);

    bestPattern.normalize();
    
    library.put( result, bestPattern );

    if (result.max_size > max_pattern_size){
      //unique_lock<mutex> _lock(stdio_mtx);
      //cerr<<"### size: "<<result.max_size<<endl;
      //max_pattern_size = result.max_size;
    };

  }else if (result.resolution == AnalysysResult::ITERATIONS_EXCEEDED){
    unique_lock<mutex> _lock(stdio_mtx);
    Pattern p(pattern);
    p.normalize();
    cerr<<"### iters exceeded for: "<<p.to_rle()<<endl;
  }else if (result.resolution == AnalysysResult::PATTERN_TO_WIDE){
    /*
    unique_lock<mutex> _lock(stdio_mtx);
    Pattern p(pattern);
    p.normalize();
    cerr<<"### too wide: "<<p.to_rle()<<endl;
    */
  }
}

void performance_reporter( PatternSource &src, Library &lib, const std::string &dump_path  )
{
  time_t timeBegin = time( nullptr );
  size_t processed = 0;
  chrono::milliseconds dura( 100 );
  size_t dump_every = 150;//every 15 sec
  size_t counter=0;

  auto do_report = [&] () -> void {
    size_t processedNow = src.get_processed();
    time_t curTime = time(NULL);
    
    double dt = difftime( curTime, timeBegin );
    if (dt>0){
      unique_lock<mutex> _lock(stdio_mtx);
      cerr << "Tthroughput: "<< ((processedNow-processed)/dt) << " rows/s"
	   << "Library size:"<< lib.get_size()
	   << endl;
    }
    processed = processedNow;
    timeBegin = curTime;
    std::ofstream lib_file(dump_path);
    lib.dump( lib_file );
  };
  
  while( ! src.is_closed() ){
    this_thread::sleep_for( dura );
    if (++ counter  != dump_every )
      continue;
    else
      counter = 0;
    do_report();
  }
  do_report();
  {
    unique_lock<mutex> _lock(stdio_mtx);
    cerr<<"finished"<<endl;
  }
}

int main(int argc, char* argv[])
{
  Library library;
  int r[] = {0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15};
  MargolusBinaryRule rule(r);
  
  cout << "Rule is: "<<rule << endl;

  istream &ifile(cin);


  //may return 0 when not able to detect
  size_t nthreads = std::thread::hardware_concurrency();
  if (nthreads == 0){
    cerr<<"Failed to determine number of cores, using 1"<<endl;
    nthreads = 1;
  }

  cerr << "Running "<<nthreads<<" threads"<<endl;

  PatternSource source(ifile);

  vector<thread> workers;
  vector<unique_ptr<Analyzer> >analyzers;
  for(size_t i=0; i!=nthreads; ++i){
    unique_ptr<Analyzer> analyzer(new TreeAnalyzer(rule));
    analyzer->max_iters = 10000;
    analyzer->max_size = 30;
    //Analyzer &analyzer, Library & lib, PatternSource& source )
    workers.push_back( thread( analysys_worker, ref( *analyzer), ref(library), ref(source) ));
    analyzers.push_back(move(analyzer));
  }
  
  cerr<<"Started threads, now waiting"<<endl;
  //performance_reporter( PatternSource &src, Library &lib )
  thread perfReporter(performance_reporter, ref(source), ref(library), "library.json" );
  
  for( auto &t: workers){
    t.join();
  }

  perfReporter.join();
  cerr<<"Finished processing"<<endl;
  
  return 0;
}
