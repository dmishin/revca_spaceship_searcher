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

class Library{
public:
  
  struct LibraryRecord{
    int count;
    int period;
    Cell offset;
  };
  typedef std::unordered_map<Pattern, LibraryRecord> catalog_t;
  std::mutex lock;
private:
  catalog_t catalog;
public:
  void put( const AnalysysResult & result, const Pattern &bestPattern );
  void dump( std::ostream &os );
  void read( std::istream &is );
};


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
};

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
  while( source.get(p, generation) ){
    analyze_record( analyzer, generation, p, lib);
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
}
void Library::read( std::istream &is )
{
  std::unique_lock<std::mutex> _locker(lock);
  //TODO
}


void parse_record( picojson::value &record, int & generation, Pattern &pattern )
{
  if ( !record.is<picojson::object>() ) throw logic_error("record is not object");

  generation = (int)record.get("g").get<double>();

  picojson::value & record_p = record.get("p");
  if (!record_p.is<picojson::array>()) throw logic_error("p is not array");
  picojson::array &cells_array = record_p.get<picojson::array>();
  size_t ncells = cells_array.size();
  for(size_t i=0; i!=ncells; ++i){
    picojson::array &xy = cells_array[i].get<picojson::array>();
    pattern.append( (int)xy[0].get<double>(),
		    (int)xy[1].get<double>() );
  }
}

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

    /*
    string rle = bestPattern.to_rle();
    cout <<"{"<<"pop:"<<bestPattern.size()
	 <<","<<"g:"<<generation
	 <<","<<"p:"<<result.period
	 <<","<<"v:"<<result.offset
	 <<","<<"rle:\""<< bestPattern.to_rle() << "\""
	 <<"}"<<endl;
    */
  }
}

void performance_reporter( PatternSource &src, Library &lib )
{
  
}

int main(int argc, char* argv[])
{
  Library library;
  int r[] = {0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15};
  MargolusBinaryRule rule(r);
  
  cout << "Rule is: "<<rule << endl;

  istream &ifile(cin);

  size_t nthreads = 2;
  cerr << "Running "<<nthreads<<" threads"<<endl;

  PatternSource source(ifile);

  vector<thread> workers;
  vector<unique_ptr<Analyzer> >analyzers;
  for(size_t i=0; i!=nthreads; ++i){
    unique_ptr<Analyzer> analyzer(new TreeAnalyzer(rule));
    //Analyzer &analyzer, Library & lib, PatternSource& source )
    workers.push_back( thread( analysys_worker, ref( *analyzer), ref(library), ref(source) ));
    analyzers.push_back(move(analyzer));
  }
  
  cerr<<"Started threads, now waiting"<<endl;
  for( auto &t: workers){
    t.join();
  }
  cerr<<"Finished OK"<<endl;
  
  
  return 0;
}
