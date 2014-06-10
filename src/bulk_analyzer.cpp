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

using namespace std;


void analyze_record( const MargolusBinaryRule &rule, picojson::value &record )
{
  if ( !record.is<picojson::object>() ) throw logic_error("record is not object");

  int generation = (int)record.get("g").get<double>();
  Pattern pattern;
  picojson::value & record_p = record.get("p");
  if (!record_p.is<picojson::array>()) throw logic_error("p is not array");
  picojson::array &cells_array = record_p.get<picojson::array>();

  size_t ncells = cells_array.size();
  for(size_t i=0; i!=ncells; ++i){
    picojson::array &xy = cells_array[i].get<picojson::array>();
    pattern.append( (int)xy[0].get<double>(),
		    (int)xy[1].get<double>() );
  }
  pattern.normalize();
  cout << " pattern: "<< pattern.to_rle() << endl;

  AnalysysResult result;
  AnalyzeOpts opts;
  analyze(pattern, rule, opts, result);
  cout << "analysys:"<<result.resolution<<endl;
  cout << "  offset:"<<result.offset<<endl;
}
int main(int argc, char* argv[])
{
  int r[] = {0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15};
  MargolusBinaryRule rule(r);

  cout << "Rule is: "<<rule << endl;


  istream &ifile(cin);
  const int buf_size = 2048;

  char line_buffer[buf_size];
  while(true){
    ifile.getline(line_buffer, buf_size);
    if (line_buffer[0]=='\0') break;
    //parsing 
    picojson::value record;
    istringstream iss(line_buffer);
    iss >> record;
    analyze_record( rule, record );
  };
  
  return 0;
}
