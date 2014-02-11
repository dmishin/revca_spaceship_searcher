#include "cpp-revca.hpp"
#include "field.hpp"
#include <vector>
#include <cstdlib>
#include <iostream>

///Scan for patterns, removing them and putting to the vector.
/// Patterns are allocated by new.
void scan_figures( MargolusBinaryField &field, const Cell &p0, const Cell &p1, std::vector<Pattern*> &patterns, int range, int max_size )
{
  for(int y=p0[1]; y<p1[1]; ++y){
    assert(y>=0 && y<field.height());
    for(int x=p0[0]; x<p1[0]; ++x){
      assert(x>=0 && y<field.width());
      if (field.get(x,y) != 0){
	Pattern * p = new Pattern;
	field.pick_pattern_at(*p, Cell(x,y), /*erase=*/true, range, max_size);
	patterns.push_back(p);
      }
    }
  }
}

void random_fill( MargolusBinaryField &fld, const Cell& p0, const Cell& p1, double percent)
{
  for(int y=p0[1]; y<p1[1]; ++y){
    for(int x=p0[0]; x<p1[0]; ++x){
      fld.set(x,y, ((double)rand()/(double)RAND_MAX <= percent) ? 1 : 0 );
    }
  }
}

void do_random_search( const MargolusBinaryRule &rule,
		       int field_size, int random_box_size,
		       double random_percent, 
		       int generations,
		       int pick_range, 
		       int max_pattern_size )
{
  MargolusBinaryField field( field_size/2, field_size/2 );
  std::vector<Pattern*> patterns;
  field.fill(0);
  int x0 = (field_size-random_box_size)/2;
  int x1 = (field_size+random_box_size)/2;
  random_fill( field, Cell(x0,x0), Cell(x1,x1), random_percent );
  for (int generation=0; generation<generations; generation+=2){
    //std::cout<<"Generation:"<<generation<<std::endl;
    field.transform2(rule);
    //d::cout<<field;
    //Scan the top-left border

    //std::cout<<"  scanning..."<<std::endl;
    scan_figures(field, Cell(0,0), Cell(field_size,2), 
		 patterns, pick_range, max_pattern_size);
    scan_figures(field, Cell(0,2), Cell(2,field_size), 
		 patterns, pick_range, max_pattern_size);
    //std::cout<<"  printing "<<patterns.size()<<"patterns"<<std::endl;
    for( Pattern *p : patterns){
      if(p->size()==0) continue;
      p->normalize();
      std::cout<<"Pattern found: "<<p->size()<<" cells "<<p->to_rle()<<std::endl;
      delete p;
    }
    patterns.clear();
  }
}
		       



struct AnalyzeOpts
{
  int max_iters; //=2048
  int max_population;//=100;
  int max_size;//1000;
};
struct  AnalysysResult{
  int analyzed_generations;
  std::string resolution;
  int period;
  Cell offset;
  Pattern bestPattern;
};

template <class T, class MeasureFunction, class Measure=double>
struct Maximizer{
  bool hasValue;
  Measure bestMeasure;
  T bestValue;
  MeasureFunction measureFunc;
  Maximizer( MeasureFunction func=MeasureFunction() )
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
    return 0;
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
    if (isOffsetEqualWithOddity( curPattern, pattern, odd(phase), offset)){
      //cycle found!
      result.resolution = "cycle found";
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

