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
#include <stdexcept>

#include "streamed_analysys.hpp"
#include "file_pattern_source.hpp"
#include "bruteforce_pattern_source.hpp"
#include "tree_pattern.hpp"

#include "optionparser.h"


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
		   const string &output_file,
		   int max_iters,
		   int max_size)
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
    analyzer->max_iters = max_iters;
    analyzer->max_size = max_size;
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

enum  optionIndex { UNKNOWN, HELP, SOURCE, BRUTEFORCE, BRUTEFORCE_START, RULE, MAX_ITERATIONS, MAX_SIZE };

const option::Descriptor usage[] =
{
 {UNKNOWN, 0, "", "",option::Arg::None, "USAGE: bulk_analyzer [options] library.json\n\n"
                                        "Options:" },
 {HELP, 0,"h", "help",option::Arg::None, 
  "  --help  \tPrint usage and exit." },
 {SOURCE, 0,"s","source",option::Arg::Optional, 
  "  -s, --source FILE.jsons\tSource file, containing patterns. Format is JSON stream, one pattern per line. Pattern is list of [x,y] pairs." },
 {BRUTEFORCE, 0, "b", "bruteforce", option::Arg::Optional,
  "  -b, --bruteforce=N Brute-force search for patterns of size N. Incompatible with --source. Search is infinite."},
 {BRUTEFORCE_START, 0, "B", "bruteforce-start", option::Arg::Optional,
  "  -B, --bruteforce-start=N1,N2,N3,... Start index of the bruteforce search. Usable for continuing searches."},
 {RULE, 0, "r", "rule", option::Arg::Optional,
  "  -r, --rule=R1,R2,...,R15 Rule, comma-separated list of integers. Default is single rotation"},
 {MAX_ITERATIONS, 0, "I", "max-iter", option::Arg::Optional,
  "  -I, --max-iter Maximal numer of iterations or analysys. Default is 10000"},
 {MAX_SIZE, 0, "S", "max-size", option::Arg::Optional,
  "  -S, --max-size Maximum size of the bounding box of the pattern. Default is 30"},


 {0,0,0,0,0,0}
};


struct Options{
  string source_file;
  int bruteforce_size;
  bool use_bruteforce;
  vector<int> bruteforce_start;
  MargolusBinaryRule rule;
  int max_iterations;
  int max_size;
  string output_file;

  Options()
    :bruteforce_size(-1)
    ,use_bruteforce(false)
    ,rule({{0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15}})
    ,max_iterations(10000)
    ,max_size(50)
  {}
  bool parse(int argc, char* argv[]);
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
    if (! (ss>>data[i]) )
      throw logic_error("Failed to parse comma-delimited list of integers");
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
    if (!use_bruteforce) throw logic_error("--briteforce and --source are incompatile. Decide what do you need.");
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
    cout<<"bfs"<<endl;
    if (!use_bruteforce) throw logic_error("Not using bruteforce, bruteforce-start meaningless");
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

  cout << "Rule is: "<<options.rule << endl;
  if (options.use_bruteforce){
    cout << "Bruteforcing patterns of size "<<options.bruteforce_size<<endl;

    BruteforceSource source(options.bruteforce_size);
    
    SinglerotCoralFilter pattern_filter;
    run_analysis( source,
		  pattern_filter,
		  library, 
		  options.rule,
		  options.output_file,
		  options.max_iterations,
		  options.max_size);
  }else{
    //processing file
    ifstream file_data(options.source_file);
    PatternSource fsource(file_data);

    NoFilter pattern_filter;
    run_analysis( fsource, 
		  pattern_filter,
		  library, 
		  options.rule,
		  options.output_file,
		  options.max_iterations,
		  options.max_size);
    
  }
  return 0;
}
