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

using namespace std;


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

    result.bestPattern.normalize();
  
    string rle = result.bestPattern.to_rle();
    cout <<"{"<<"pop:"<<result.bestPattern.size()
	 <<","<<"g:"<<generation
	 <<","<<"p:"<<result.period
	 <<","<<"v:"<<result.offset
	 <<","<<"rle:\""<< result.bestPattern.to_rle() << "\""
	 <<"}"<<endl;
  }
}
int main(int argc, char* argv[])
{
  int r[] = {0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15};
  MargolusBinaryRule rule(r);
  Analyzer analyzer(rule);
  
  cout << "Rule is: "<<rule << endl;


  istream &ifile(cin);
  const int buf_size = 2048;
  //const size_t min_pattern_size = 7;
  //size_t max_cache = 10;
  
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
    if (dt > 1.0){
      //double miss_ratio = (double)analyzer.cache_misses / (double)(analyzer.cache_hits + analyzer.cache_misses);
      cerr << "Tthroughput: "<< (processed/dt) << " rows/s"
//	   << " miss/hit ratio:" << miss_ratio
	   << endl;
      processed = 0;
      timeBegin = curTime;
    }
  };
  
  return 0;
}
