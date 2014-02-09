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
/*
void analyze(const Pattern &pattern_, const MargolusBinaryRule &rule, 
	     const AnalyzeOpts &options) {
  var bestPatternSearch, bounds, cells_best, curPattern, cycle_found, dx, dy, iter, max_iters, max_population, max_size, offsetToOrigin, phase, result, snap_below, stable_rule, stable_rules, vacuum_period, x0, y0, _i, _j, _len, _ref, _ref1, _ref2, _ref3, _ref4;

  int max_iters = options.max_iters;
  int max_population = options.max_population;
  int max_size = options.max_size;
  auto snap_below = [](int x, int generation) -> int{
    return x - mod(x + generation, 2);
  };
  auto offsetToOrigin = [](Pattern &pattern, bounds, generation) {
      auto xy0 = pattern.top_left();
      int x0 = snap_below(xy0[0], generation);
      int y0 = snap_below(xy0[1], generation);
      pattern.offset(pattern, -x0, -y0);
      return std::make_tuple(pattern, x0, y0);
  };
  Rule stable_rules[] = {rule};
  int vacuum_period = 1;//stable_rules.length;
  Pattern pattern(pattern_);
  pattern.normalize();
  pattern = offsetToOrigin(pattern, Cells.bounds(pattern), 0)[0];
    bestPatternSearch = new Maximizer(this.energy);
    bestPatternSearch.put(pattern);
    cycle_found = false;
    curPattern = pattern;
    dx = 0;
    dy = 0;
    result = {
      analyzed_generations: max_iters,
      resolution: "iterations exceeded"
    };
    for (iter = _i = vacuum_period; vacuum_period > 0 ? _i <= max_iters : _i >= max_iters; iter = _i += vacuum_period) {
      phase = 0;
      for (_j = 0, _len = stable_rules.length; _j < _len; _j++) {
        stable_rule = stable_rules[_j];
        curPattern = evaluateCellList(stable_rule, curPattern, phase);
        phase ^= 1;
      }
      this.sortXY(curPattern);
      bounds = Cells.bounds(curPattern);
      _ref3 = offsetToOrigin(curPattern, bounds, phase), curPattern = _ref3[0], x0 = _ref3[1], y0 = _ref3[2];
      dx += x0;
      dy += y0;
      if (this.areEqual(pattern, curPattern)) {
        cycle_found = true;
        result.resolution = "cycle found";
        break;
      }
      bestPatternSearch.put(curPattern);
      if (curPattern.length > max_population) {
        result.resolution = "pattern grew too big";
        break;
      }
      if (Math.max(bounds[2] - bounds[0], bounds[3] - bounds[1]) > max_size) {
        result.resolution = "pattern dimensions grew too big";
        break;
      }
    }
    cells_best = bestPatternSearch.getArg();
    if (cycle_found) {
      _ref4 = this.canonicalize_spaceship(cells_best, rule, dx, dy), cells_best = _ref4[0], result.dx = _ref4[1], result.dy = _ref4[2];
      result.period = iter;
    }
    result.cells = cells_best;
    return result;
  },
*/
