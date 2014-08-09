//#include "gason.hpp"
#include "rule.hpp"
#include "pattern.hpp"
#include "analyze.hpp"

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <ctime>
#include <unordered_map>

#include <thread>
#include <mutex>
#include <stdexcept>

#include "streamed_analysys.hpp"
#include "file_pattern_source.hpp"
#include "bruteforce_pattern_source.hpp"
#include "tree_pattern.hpp"
#include "singlerot.hpp"

#include "optionparser.h"

using namespace std;

struct Options{
  string source_file;
  int bruteforce_size;
  bool use_bruteforce;
  vector<int> bruteforce_start;
  MargolusBinaryRule rule;
  int max_iterations;
  int max_size;
  string output_file;
  size_t threads;
  int dump_interval_ms;

  Options()
    :bruteforce_size(-1)
    ,use_bruteforce(false)
    ,rule(singlerot)
    ,max_iterations(10000)
    ,max_size(50)
    ,threads(0)
    ,dump_interval_ms(1000)
  {}
  bool parse(int argc, char* argv[]);
};

void analyze_record( Analyzer &analyzer, int generation, const Pattern &pattern, Library &library );

std::mutex stdio_mtx;

void analysys_worker( Analyzer &analyzer, Library & lib, AbstractPatternSource& source)
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
      cerr<<"  error processing pattern: "<<err.what()<<endl
	  <<"  pattern is:"<<p<<endl;
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
    cerr<<"  iterations exceeded: "<<p.to_rle()<<endl;
  }else if (result.resolution == AnalysysResult::PATTERN_TO_WIDE){
    /*
    unique_lock<mutex> _lock(stdio_mtx);
    Pattern p(pattern);
    p.normalize();
    cerr<<"### too wide: "<<p.to_rle()<<endl;
    */
  }
}

void performance_reporter( AbstractPatternSource &src, Library &lib, const Options &opt  )
{
  time_t timeBegin = time( nullptr );
  size_t processed = 0;
  const int wake_every = 100;
  chrono::milliseconds dura( wake_every );
  size_t dump_every = (max)(1, opt.dump_interval_ms / wake_every);
  size_t counter=0;

  auto do_report = [&] () -> void {
    size_t processedNow = src.get_processed();
    time_t curTime = time(nullptr);
    
    double dt = difftime( curTime, timeBegin );
    if (dt>0){
      unique_lock<mutex> _lock(stdio_mtx);
      cout << "Throughput: "<< ((processedNow-processed)/dt) << " patterns/s" << endl
	   << "Library size:"<< lib.get_size() << endl
	   << "Current position:"<< src.get_position_text() <<endl<<endl;
      processed = processedNow;
      timeBegin = curTime;
    }
    ofstream lib_file(opt.output_file);
    lib.dump( lib_file );
  };
  
  while( ! src.is_closed() ){
    this_thread::sleep_for( dura );
    if (++ counter  == dump_every ){
      counter = 0;
      do_report();
    }
  }
  do_report();
  {
    unique_lock<mutex> _lock(stdio_mtx);
    cout<<"Reporter thread finished"<<endl;
  }
}



void run_analysis( AbstractPatternSource &source, 
		   Library &library, 
		   const Options &options)
{
  //may return 0 when not able to detect

  size_t nthreads = options.threads;
  if (nthreads == 0){ //not specified.
    nthreads = thread::hardware_concurrency();
  }
  if (nthreads == 0){
    nthreads = 1;
  }

  cerr << "Running "<<nthreads<<" analysys threads"<<endl;

  vector<thread> workers;
  vector<unique_ptr<Analyzer> >analyzers;
  for(size_t i=0; i!=nthreads; ++i){
    unique_ptr<Analyzer> analyzer(new TreeAnalyzer(options.rule));
    analyzer->max_iters = options.max_iterations;
    analyzer->max_size = options.max_size;
    //Analyzer &analyzer, Library & lib, PatternSource& source )
    workers.push_back( thread( analysys_worker, ref( *analyzer), ref(library), ref(source) ));
    analyzers.push_back(move(analyzer));
  }
  
  cerr<<"Started threads, now waiting for termination"<<endl;

  thread perfReporter(performance_reporter, ref(source), ref(library), ref(options));
  
  for( auto &t: workers){
    t.join();
  }

  perfReporter.join();
  cerr<<"Finished processing"<<endl;
}

enum  optionIndex { UNKNOWN, HELP, SOURCE, BRUTEFORCE, BRUTEFORCE_START, RULE, MAX_ITERATIONS, MAX_SIZE, THREADS, DUMP_INTERVAL_MS };

const option::Descriptor usage[] =
{
 {UNKNOWN, 0, "", "",option::Arg::None, "USAGE: bulk_analyzer [options] library.json\n\n"
                                        "Options:" },
 {HELP, 0,"h", "help",option::Arg::None, 
  "  --help  \tPrint usage and exit." },
 {SOURCE, 0,"s","source",option::Arg::Optional, 
  "  -s, --source FILE.jsons Source file, containing patterns. Format is JSON stream, one pattern per line. Pattern is list of [x,y] pairs." },
 {BRUTEFORCE, 0, "b", "bruteforce", option::Arg::Optional,
  "  -b, --bruteforce=N Brute-force search for patterns of size N. Incompatible with --source. Search is infinite."},
 {BRUTEFORCE_START, 0, "B", "bruteforce-start", option::Arg::Optional,
  "  -B, --bruteforce-start=N1,N2,N3,... Start index of the bruteforce search. Usable for continuing searches."},
 {RULE, 0, "r", "rule", option::Arg::Optional,
  "  -r, --rule=R1,R2,...,R16 Rule, comma-separated list of integers. Default is single rotation"},
 {MAX_ITERATIONS, 0, "I", "max-iter", option::Arg::Optional,
  "  -I, --max-iter Maximal numer of iterations or analysys. Default is 10000"},
 {MAX_SIZE, 0, "S", "max-size", option::Arg::Optional,
  "  -S, --max-size Maximum size of the bounding box of the pattern. Default is 30"},

 {THREADS, 0, "T", "threads", option::Arg::Optional,
  "  -T, --threads Number of analysys threads. Default is number of processors"},
 {DUMP_INTERVAL_MS, 0, "", "dump-interval", option::Arg::Optional,
  " --dump-interval=N Update library every N milliseconds. Default is 1000 (1s)"},
 {0,0,0,0,0,0}
};

const char * null_to_empty( const char * s )
{
  if (! s) return "";
  else return s;
}
void parse_comma_list( const char *slist, int *data, size_t size)
{
  stringstream ss(slist);
  for( size_t i=0; i<size; ++i){
    if (! (ss>>data[i]) ){
      stringstream msg;
      msg <<"Expected comma-delimited list of "
	  << size << " integers, but got:"<<slist;
      throw logic_error(msg.str());
    }
    if (ss.peek() == ',')
	ss.ignore();
  }
}

bool Options::parse(int argc, char* argv[])
{
  if (argc > 0){ //skip program name
    argc --;
    argv ++;
  };
  option::Stats  stats(usage, argc, argv);
  option::Option* options = new option::Option[stats.options_max];
  option::Option* buffer  = new option::Option[stats.buffer_max];
  option::Parser parse(usage, argc, argv, options, buffer);
  if (parse.error())
    throw invalid_argument("Failed to parse options");

  if (options[HELP] || argc == 0) {
    option::printUsage(cout, usage);
    return false;
  }
  
  use_bruteforce = true;
  if (options[SOURCE]){
    source_file = options[SOURCE].last()->arg;
    use_bruteforce = false;
  }
  if (options[BRUTEFORCE]){
    if (!use_bruteforce) throw logic_error("--bruteforce and --source are incompatile. Decide what do you need.");
    use_bruteforce=true;
    stringstream ss(null_to_empty(options[BRUTEFORCE].last()->arg));
    if (!(ss >> bruteforce_size)) throw logic_error("Faield to parse bruteforce size");
    bruteforce_start.resize( bruteforce_size-1 );
    fill( bruteforce_start.begin(), bruteforce_start.end(), 0 );
  }else{
    if (use_bruteforce)
      throw logic_error("Either source file or bruteforce must be specified");
  }
  
  if (options[BRUTEFORCE_START]){
    if (!use_bruteforce) 
      throw logic_error("Not using bruteforce, --bruteforce-start meaningless");
    parse_comma_list( null_to_empty( options[BRUTEFORCE_START].last()->arg),
		      &(bruteforce_start[0]),
		      bruteforce_start.size());
    int prev=0;
    for( int si: bruteforce_start ){
      if (si < prev) throw logic_error("Start index must consist of increasing integers");
      prev = si;
    }
  }
  if (options[RULE]){
    int irule[16];
    parse_comma_list( null_to_empty( options[RULE].last()->arg),
		      irule,
		      16 );
    rule = MargolusBinaryRule( irule );
    if (rule(0) != 0)
      throw logic_error("Rule changes empty field, analysy not supported");
      
  }
  if (options[MAX_SIZE]){
    stringstream ss( null_to_empty(options[MAX_SIZE].last()->arg));
    if (!(ss>>max_size))
      throw logic_error("can't parse max size");
    if (max_size<=0)
      throw logic_error("max_size must be positive");
  }
  if (options[MAX_ITERATIONS]){
    stringstream ss( null_to_empty(options[MAX_ITERATIONS].last()->arg));
    if (!(ss>>max_iterations))
      throw logic_error("can't parse max iters");
    if (max_iterations<=0)
      throw logic_error("max_iters must be positive");
  }
  if (options[THREADS]){
    stringstream ss( null_to_empty(options[THREADS].last()->arg));
    if (!(ss>>threads))
      throw logic_error("Failed to parse number of threads");
    if (threads <0 || threads>1000){
      stringstream msg;
      msg << "Wrong number of threads:"<<threads;
      throw logic_error(msg.str());
    }
  }
  if (options[DUMP_INTERVAL_MS]){
    stringstream ss( null_to_empty(options[DUMP_INTERVAL_MS].last()->arg));
    if (!(ss>>dump_interval_ms))
      throw logic_error("Failed to parse dump interval value");
    if (dump_interval_ms <= 0){
      stringstream msg;
      msg << "Wrong dump interval:"<<dump_interval_ms<<"ms";
      throw logic_error(msg.str());
    }
  }
  //parse output file
  if (parse.nonOptionsCount() < 1)
    throw logic_error("library name not specified");
  if (parse.nonOptionsCount() > 1)
    throw logic_error("redundant arguments after library name");
  output_file = parse.nonOption(0);
  return true;
}

int main(int argc, char* argv[])
{

  Options options;
  try{
    if (!options.parse(argc, argv))
      return 0;
    cout<<"Parsed options OK"<<endl;
  }catch(exception &e){
    cerr<<"Error: "<<e.what()<<endl;
    return 1;
  }

  Library library;

  cout << "Rule is: "<<options.rule << endl
       << "Writing library to "<<options.output_file<<endl;

  if (options.use_bruteforce){
    library.store_hit_count = false; //statistics useless when bruteforcing.
    cout << "Bruteforcing patterns of size "<<options.bruteforce_size<<endl;

    BruteforceSource source(options.bruteforce_start);

    if (options.rule == singlerot){
      cout<<" SingleRotation rule used, enabling filter for indestructible patterns"<<endl;

      unique_ptr<SinglerotCoralFilter> pattern_filter(new SinglerotCoralFilter);
      source.add_filter(move(pattern_filter));
    }
    run_analysis( source,
		  library, 
		  options);
  }else{
    //processing file
    ifstream file_data(options.source_file);
    PatternSource fsource(file_data);
    
    run_analysis( fsource, 
		  library, 
		  options);
    
  }
  return 0;
}
