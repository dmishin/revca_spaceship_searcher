#include "analyze.hpp"
#include "cpp-revca.hpp"
#include "field.hpp"
#include "random_search.hpp"
#include "mathutil.hpp"

#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <tuple>
#include <stdexcept>
#include <cstdlib>

using namespace std;


double pattern_energy( const Pattern &p )
{
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
}

void analyze(const Pattern &pattern_, const MargolusBinaryRule &rule, 
	     const AnalyzeOpts &options,
	     AnalysysResult &result) {
  Analyzer analyzer(rule);
  (AnalyzeOpts&)(analyzer) = options; //write options inside
  result = analyzer.process( pattern_ );
}


const Transform rotations[4] =
  { Transform( 1,0,0,1 ),
    Transform( 0,-1,1,0),
    Transform( -1,0,0,-1),
    Transform( 0,1,-1,0) };

const Transform & normalizing_rotation( const Cell &offset )
{
  if (offset==Cell(0,0)) return rotations[0];

  for( int i=0; i<4; ++i){
    Cell offset1 = rotations[i](offset);
    if (offset1[0] > 0 && offset1[1] >= 0)
      return rotations[i];
  }
  throw logic_error("impossible situation");
}


AnalyzerCache::AnalyzerCache()
{}

size_t AnalyzerCache::put( const AnalysysResult &result )
{
  unique_ptr<AnalysysResult> presult(new AnalysysResult(result));
  results.push_back(move( presult ) );
  return results.size()-1;
}

AnalysysResult *AnalyzerCache::get_cached( const Pattern &p )
{
  auto found = cache.find(p);
  if (found != cache.end())
    return found -> second;
  else
    return nullptr;
}
void AnalyzerCache::put( const Pattern &key, size_t result_index )
{
  cache[key] = results[result_index].get();
}


struct EnergyFunc{
  double operator ()(const std::pair<Pattern, int> &p_phase)const{
    return pattern_energy(p_phase.first); 
  };
};
AnalysysResult Analyzer::process( const Pattern &pattern_)
{
  AnalysysResult result;
  Maximizer<pair<Pattern, int>, EnergyFunc, double> bestPatternSearch;

  MargolusBinaryRule stable_rules[] = {rule};

  int vacuum_period = 1;//stable_rules.length;

  int phase = 0;

  on_start_processing( pattern_ );
  Pattern pattern(pattern_);
  
  pattern.normalize();
  bestPatternSearch.put(make_pair(pattern, phase) ); //initial phase is 0

  Pattern curPattern(pattern);

  result. analyzed_generations = max_iters;
  result. resolution = AnalysysResult::ITERATIONS_EXCEEDED;
  result.period = -1;

  Cell offset;
  for (int iter = vacuum_period; iter <= max_iters; iter += vacuum_period) {
    for (int irule=0;irule<vacuum_period;++irule) {
      evaluateCellList(stable_rules[irule], curPattern, phase, curPattern);
      phase ^= 1;
    }
    curPattern.sort();
    on_iteration( iter, curPattern );
    if (isOffsetEqualWithOddity( pattern, curPattern, odd(phase), offset)){
      //cycle found!
      result.resolution = AnalysysResult::CYCLE_FOUND;
      result.period = iter;
      //normalizing rotation of the spaceship
      const Transform &t = normalizing_rotation( offset );
      bestPatternSearch.getBestValue().first.transform( t, result.bestPattern );
      int bestValuePhase = bestPatternSearch.getBestValue().second;
      result.bestPattern.translate(bestValuePhase,bestValuePhase);
      result.bestPattern.normalize();
      result.offset = t(offset);
      on_result_found( pattern_, result );
      return result;
    }
    bestPatternSearch.put(make_pair(curPattern, phase));
    if (curPattern.size() > (size_t)max_population) {
      result.resolution = AnalysysResult::PATTERN_TOO_BIG;
      break;
    }
    auto bounds = pattern.bounds();
    Cell size = get<1>(bounds) - get<0>(bounds);
    if (max(abs(size[0]), abs(size[1])) > max_size){
      result.resolution = AnalysysResult::PATTERN_TO_WIDE;
      break;
    }
  }
  //search for cycle finished
  result.bestPattern = bestPatternSearch.getBestValue().first;
  int bestValuePhase = bestPatternSearch.getBestValue().second;
  result.bestPattern.translate(bestValuePhase,bestValuePhase);
  result.bestPattern.normalize();

  result.offset = Cell(0,0);
  on_result_found( pattern_, result );
  return result;
}

void CachingAnalyzer::on_start_processing( const Pattern &pattern )
{
  using namespace std;
  if (cache_frozen) return;
  evolution.clear();
  unique_ptr<Pattern> ppattern(new Pattern(pattern));
  evolution.push_back(move(ppattern));
}

void CachingAnalyzer::on_iteration( int age, const Pattern &pattern )
{
  using namespace std;
  if (cache_frozen) return;
  unique_ptr<Pattern> ppattern(new Pattern(pattern));
  evolution.push_back(move(ppattern));
}

void CachingAnalyzer::on_result_found( const Pattern &pattern, const AnalysysResult &result)
{
  using namespace std;
  if (cache_frozen) return;
  if (result.resolution == AnalysysResult::CYCLE_FOUND){
    size_t result_index = cache.put( result );
    for( unique_ptr<Pattern> &ppattern: evolution ){
      //TODO: actually, all possible rotations must be put!
      Pattern p(*ppattern);
      p.normalize();
      cache.put( p, result_index );
    }
  }
  evolution.clear();
}

AnalysysResult CachingAnalyzer::process( const Pattern &pattern)
{
  using namespace std;
  Pattern normalized_pattern(pattern);
  normalized_pattern.normalize();
  AnalysysResult * presult = cache.get_cached(normalized_pattern);
  
  if( presult != nullptr ){
    cache_hits ++;
    return *presult;
  }else{
    cache_misses ++;
    return Analyzer::process( pattern );
  }
}
