#include "file_pattern_source.hpp"
#include "picojson.h"
#include <sstream>

using namespace std;

void parse_record( picojson::value &record, int & generation, Pattern &pattern );

bool PatternSource::get_nofilter( Pattern & p, int &generation )
{
  picojson::value record;
  {
    if (closed) return false;
    stream.getline(line_buffer, buf_size);
    if (line_buffer[0]=='\0') {
      closed = true;
      return false;
    }
    lineno ++;
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


std::string PatternSource::get_position_text()const
{
  std::unique_lock<std::mutex> _lock_stream(lock);
  stringstream ss;
  ss<<"line #"<<lineno;
  return ss.str();
}
