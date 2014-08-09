#ifndef __FILE_PATTERN_SOURCE_INCLUDED__
#define __FILE_PATTERN_SOURCE_INCLUDED__

#include "streamed_analysys.hpp"

/**Pattern producer, reading patterns from the file, generated by the standalone searched.
 */
class PatternSource: public AbstractPatternSource{
  std::istream &stream;
  static const size_t buf_size=2048;
  size_t lineno;
  char line_buffer[buf_size];
protected:
  virtual bool get_nofilter( Pattern & p, int& g );
public:
  PatternSource( std::istream &s ): stream(s), lineno(0){};
  virtual std::string get_position_text()const;
};



#endif
