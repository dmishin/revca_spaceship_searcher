//#include "gason.hpp"
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

#include "streamed_analysys.hpp"
#include "file_pattern_source.hpp"
#include "bruteforce_pattern_source.hpp"
#include "tree_pattern.hpp"

using namespace std;

void analyze_record( Analyzer &analyzer, int generation, const Pattern &pattern, Library &library );

std::mutex stdio_mtx;

void analysys_worker( Analyzer &analyzer, Library & lib, AbstractPatternSource& source, PatternFilter &filter )
{
  Pattern p;
  int generation;
  while(true){
    try{
      if (! source.get(p, generation) )
	break;
      if (filter.check(p)){
	//Pattern p1(p);
	//p1.normalize();
	//cerr<<"Pattern not filtered out:"<<p1.to_rle()<<endl;

	analyze_record( analyzer, generation, p, lib);

      }else{
	//Pattern p1(p);
	//p1.normalize();
	//cerr<<"Pattern filtered out:"<<p1.to_rle()<<endl;
      }
    }catch(std::exception &err){
      unique_lock<mutex> _lock(stdio_mtx);
      cerr<<"#Error parsing line: ["<<err.what()<<"]"<<endl;
    }
    p.clear();
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

void performance_reporter( AbstractPatternSource &src, Library &lib, const std::string &dump_path  )
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
      cerr << "Throughput: "<< ((processedNow-processed)/dt) << " rows/s"
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



void run_analysis( AbstractPatternSource &source, 
		   PatternFilter &filter,
		   Library &library, 
		   MargolusBinaryRule &rule,
		   const string &output_file)
{
  //may return 0 when not able to detect
  size_t nthreads = std::thread::hardware_concurrency();
  if (nthreads == 0){
    cerr<<"Failed to determine number of cores, using 1"<<endl;
    nthreads = 1;
  }

  cerr << "Running "<<nthreads<<" threads"<<endl;

  vector<thread> workers;
  vector<unique_ptr<Analyzer> >analyzers;
  for(size_t i=0; i!=nthreads; ++i){
    unique_ptr<Analyzer> analyzer(new TreeAnalyzer(rule));
    analyzer->max_iters = 20000;
    analyzer->max_size = 40;
    //Analyzer &analyzer, Library & lib, PatternSource& source )
    workers.push_back( thread( analysys_worker, ref( *analyzer), ref(library), ref(source), ref(filter) ));
    analyzers.push_back(move(analyzer));
  }
  
  cerr<<"Started threads, now waiting"<<endl;
  //performance_reporter( PatternSource &src, Library &lib )
  thread perfReporter(performance_reporter, ref(source), ref(library), output_file);
  
  for( auto &t: workers){
    t.join();
  }

  perfReporter.join();
  cerr<<"Finished processing"<<endl;
}

int main1(int argc, char* argv[])
{
  Library library;
  MargolusBinaryRule rule({0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15});
  
  cout << "Rule is: "<<rule << endl;

  PatternSource fsource(cin);

  NoFilter pattern_filter;
  run_analysis( fsource, 
		pattern_filter,
		library, 
		rule,
		"library.json");
  
  return 0;
}

bool singlerot_has_unchanged_core( const Pattern & _p )
{
  TreePattern pattern(_p);
  while (! pattern.empty() ){
    vector<Cell> keys_to_delete;
    for( auto &key_value : pattern.blocks ){
      int block = key_value.second.value;
      if (block == 1 || block == 2 || block == 4 || block == 8){ //single-cellers
	keys_to_delete.push_back( key_value.first );
      }
    }
    if (keys_to_delete.empty())
      break;
    for( const Cell &key : keys_to_delete ){
      pattern.blocks.erase( key );
    }
    pattern.translate(1,1);
  }
  
  return ! pattern.empty();
}

class SinglerotCoralFilter: public PatternFilter{
public:
  virtual bool check( const Pattern &p){ 
    return ! singlerot_has_unchanged_core(p);
  };
};
int main(int argc, char* argv[])
{
  Library library;
  MargolusBinaryRule rule({0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15});

  int pattern_size = 10;
  cout << "Bruteforcing patterns of size "<<pattern_size<<endl;
  cout << "Rule is: "<<rule << endl;

  BruteforceSource source(pattern_size);
  SinglerotCoralFilter pattern_filter;
  run_analysis( source,
		pattern_filter,
		library, 
		rule,
		"bruteforce-library.json");
  
  return 0;
}
