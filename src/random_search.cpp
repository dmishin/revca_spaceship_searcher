#include "cpp-revca.hpp"
#include "field.hpp"
#include <vector>
#include <cstdlib>

///Scan for patterns, removing them and putting to the vector.
/// Patterns are allocated by new.
void scan_figures( MargolusBinaryField &field, const Cell &p0, const Cell &p1, std::vector<Pattern*> patterns, int range, int max_size )
{
  for(int y=p0[1]; y<p1[1]; ++y){
    for(int x=p0[0]; y<p1[0]; ++x){
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
    for(int x=p0[0]; y<p1[0]; ++x){
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
    field.transform2(rule);
    //Scan the top-left border
    scan_figures(field, Cell(0,0), Cell(field_size,2), 
		 patterns, pick_range, max_pattern_size);
    scan_figures(field, Cell(0,2), Cell(2,max_pattern_size), 
		 patterns, pick_range, max_pattern_size);
    for( Pattern *p : patterns){
      p->normalize();
      std::cout<<"Pattern found: "<<p;
      delete p;
    }
  }
}
		       
