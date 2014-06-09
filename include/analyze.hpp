#ifndef _ANALYZE_HPP_INCLUDED
#define _ANALYZE_HPP_INCLUDED

#include "pattern.hpp"

struct AnalyzeOpts
{
  int max_iters; //=2048
  int max_population;//=100;
  int max_size;//1000;
};

struct AnalysysResult{
  int analyzed_generations;
  std::string resolution;
  int period;
  Cell offset;
  Pattern bestPattern;
};

class MargolusBinaryRule;

void analyze(const Pattern &pattern_, const MargolusBinaryRule &rule, 
	     const AnalyzeOpts &options,
	     AnalysysResult &result);


#endif
