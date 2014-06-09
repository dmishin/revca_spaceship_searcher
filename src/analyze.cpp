#include "analyze.hpp"
#include "cpp-revca.hpp"
#include "field.hpp"
#include "random_search.hpp"

#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <tuple>


template <class T, class MeasureFunction, class Measure=double>
struct Maximizer{
  bool hasValue;
  Measure bestMeasure;
  T bestValue;
  MeasureFunction measureFunc;
  explicit Maximizer( MeasureFunction func=MeasureFunction() )
    :hasValue(false)
    ,measureFunc(func)
  {};

  void put( const T& value){
    Measure m = measureFunc(value);
    if (m > bestMeasure){
      bestMeasure = m;
      bestValue = value;
    }
    hasValue = true;
  };
  const T& getBestValue()const{
    if (!hasValue)  throw std::logic_error("No values to choose from");
    return bestValue;
  }
};

struct EnergyFunc{
  double operator ()(const Pattern &p)const{
    using namespace std;
    int n = (int)p.size();
    double e = 0;
    for (int i = 0; i < n; ++i) {
      const Cell & c1 = p.points[i];
      for (int j = i+1; j < n; ++j) {
	Cell d = p.points[j] - c1;
        e += 1.0 / sqrt(sqrt(d[0]*d[0]+d[1]*d[1]));
      }
    }
    
    auto bounds = p.bounds();
    Cell size = get<1>(bounds) - get<0>(bounds);
    return e / ((size[0] + 1) * (size[1] + 1));
  };
};

void analyze(const Pattern &pattern_, const MargolusBinaryRule &rule, 
	     const AnalyzeOpts &options,
	     AnalysysResult &result) {

  int max_iters = options.max_iters;
  int max_population = options.max_population;
  //int max_size = options.max_size;

  Maximizer<Pattern, EnergyFunc, double> bestPatternSearch;

  MargolusBinaryRule stable_rules[] = {rule};

  int vacuum_period = 1;//stable_rules.length;
  Pattern pattern(pattern_);
  pattern.normalize();

  bestPatternSearch.put(pattern);

  Pattern curPattern(pattern);

  result. analyzed_generations = max_iters;
  result. resolution = "iterations exceeded";
  result.period = -1;
  

  Cell offset;
  int phase = 0;
  for (int iter = vacuum_period; iter <= max_iters; iter += vacuum_period) {
    for (int irule=0;irule<vacuum_period;++irule) {
      evaluateCellList(stable_rules[irule], curPattern, phase, curPattern);
      phase ^= 1;
    }
    curPattern.sort();
    //std::cout<<"#### eval:"<<iter<<" rle:"<<curPattern<<std::endl;
    if (isOffsetEqualWithOddity( pattern, curPattern, odd(phase), offset)){
      //cycle found!
      result.resolution = "period found";
      result.period = iter;
      result.offset = offset;
      break;
    }
    bestPatternSearch.put(curPattern);
    if (curPattern.size() > (size_t)max_population) {
      result.resolution = "pattern grew too big";
      break;
    }
  }
  //search for cycle finished
  result.bestPattern = bestPatternSearch.getBestValue();
}

