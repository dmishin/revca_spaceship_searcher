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


using namespace std;

class Library{
public:
  
  struct LibraryRecord{
    int count;
    int period;
    Cell offset;
  };
  typedef std::unordered_map<Pattern, LibraryRecord> catalog_t;
private:
  catalog_t catalog;
public:
  void put( const AnalysysResult & result );
  void dump( std::ostream &os );
  void read( std::istream &is );
};


void Library::put( const AnalysysResult & result )
{
  auto iitem = catalog.find( result.bestPattern );
  if (iitem != catalog.end() ){
    //already have it
    iitem->second.count ++;
  }else{
    LibraryRecord rcd;
    rcd.count = 1;
    rcd.period = result.period;
    rcd.offset = result.offset;
    catalog[result.bestPattern] = rcd;
  }
}
void Library::dump( std::ostream &os )
{
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
  //TODO
}

Library library;

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

void analyze_record( Analyzer &analyzer, int generation, const Pattern &pattern )
{
  auto result = analyzer.process(pattern);
  
  if (result.resolution == AnalysysResult::CYCLE_FOUND &&
      result.offset != Cell(0,0)){

    //find the compact representation
    //result.bestPattern = most_compact_form( result.bestPattern, result.period, analyzer.get_rule());    
    
    library.put( result );

    /*
    string rle = result.bestPattern.to_rle();
    cout <<"{"<<"pop:"<<result.bestPattern.size()
	 <<","<<"g:"<<generation
	 <<","<<"p:"<<result.period
	 <<","<<"v:"<<result.offset
	 <<","<<"rle:\""<< result.bestPattern.to_rle() << "\""
	 <<"}"<<endl;
    */
  }
}

int main(int argc, char* argv[])
{
  int r[] = {0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15};
  MargolusBinaryRule rule(r);
  TreeAnalyzer analyzer(rule);
  
  cout << "Rule is: "<<rule << endl;


  istream &ifile(cin);
  const int buf_size = 2048;
  //const size_t min_pattern_size = 7;
  //size_t max_cache = 10;
  const double update_time = 5.0;
  
  char line_buffer[buf_size];
  time_t timeBegin = time( nullptr );
  int processed = 0;
  while(true){
    ifile.getline(line_buffer, buf_size);
    if (line_buffer[0]=='\0') break;
    //parsing 
    picojson::value record;
    istringstream iss(line_buffer);
    iss >> record;

    int generation;
    Pattern pattern;
    parse_record( record, generation, pattern );
    analyze_record( analyzer, generation, pattern);
    /*
    if ((!analyzer.is_frozen()) && analyzer.cache_size() > max_cache){
      cerr << "### Cache grew to "<<analyzer.cache_size()<<" freezing it"<<endl;
      analyzer.freeze();
    }
    */
    processed ++;
    time_t curTime = time(NULL);
    double dt = difftime( curTime, timeBegin );
    if (dt > update_time){
      cerr << "Tthroughput: "<< (processed/dt) << " rows/s"
	   << endl;
      processed = 0;
      timeBegin = curTime;
      {
	std::ofstream lib_file("library.json");
	library.dump( lib_file );
      }
    }
  };
  
  return 0;
}
