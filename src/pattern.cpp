#include "pattern.hpp"
#include <algorithm>
#include "cpp-revca.hpp"
#include "field.hpp"

using namespace std;

std::ostream & operator <<(std::ostream &os, const Pattern &p)
{

  os << "[";
  bool first = true;
  for( const Cell &point : p.points ){
    if (first) first=false; else os << ",";
    os << point;
  }
  os << "]";
  return os;
}

void Pattern::sort()
{
  std::sort(points.begin(), points.end());
}

Pattern & Pattern::append( int x, int y)
{
  points.push_back(Cell(x,y));
  return *this;
}

void Pattern::translate( const Cell & to )
{
  for( Cell &p : points ){
    p += to;
  }
}

tuple<Cell, Cell> Pattern::bounds()const
{
  auto bx = coord_range(0);
  auto by = coord_range(1);
  return make_tuple( Cell( get<0>(bx), get<0>(by) ),
		     Cell( get<1>(bx), get<1>(by) ) );
}
    

void Pattern::put_to(MargolusBinaryField &fld, int x0, int y0, const Transform &tfm )const
{
  for( const Cell &c : points ){
    Cell tc=tfm(c) + Cell(x0,y0);
    fld.set(tc[0], tc[1], 1);
  }
}

std::tuple<int, int> Pattern::coord_range(int coord_index)const
{
  int xmin, xmax;
  bool first = true;
  for( const Cell & p : points ){
    int coord = p[coord_index];
    if (first)
      xmin = xmax = coord;
    else{
      xmin = std::min(xmin, coord);
      xmax = std::max(xmax, coord);
    }      
  }
  return std::make_tuple(xmin, xmax);
}

Cell Pattern::top_left()const
{
  Cell rval;
  bool first=true; 
  for( const Cell &c : points ){
    if (first){
      rval = c;
      first=false;
    }else{
      rval[0] = (min)(rval[0], c[0]);
      rval[1] = (min)(rval[1], c[1]);
    }
  }
  return rval;
}
Cell Pattern::top_left_even()const
{
  Cell tl = top_left();
  return tl - Cell(mod(tl[0],2), mod(tl[1],2));
}

void Pattern::normalize()
{
  translate( -top_left_even() );
  sort();
}
