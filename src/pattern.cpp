#include "pattern.hpp"
#include <algorithm>
#include "cpp-revca.hpp"
#include "field.hpp"
#include <sstream>
#include <map>
#include <tuple>
#include <stdexcept>
#include "mathutil.hpp"

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

//Convert list of alive cells to RLE. List of cells must be sorted by Y, then by X, and coordinates of origin must be at (0,0)
std::string Pattern::to_rle()const
{
    //COnvert sorted (by y) list of alive cells to RLE encoding
  std::stringstream rle;
  int count = 0;
    
  auto appendNumber = [&rle](int n, char c)->void{
    if (n > 1) rle << n;
    rle << c;
  };

  auto endWritingBlock = [&appendNumber, &count]()->void{
    if (count > 0){
      appendNumber(count, 'o');
      count = 0;
    }
  };

  int x = -1;
  int y = 0;
 
  for (const Cell &xyi: points){
    int xi = xyi[0], yi=xyi[1];
    int dy = yi - y;
    if (dy < 0)
      throw std::logic_error("Cell list are not sorted by Y");
      
    if (dy > 0){ //different row
      endWritingBlock();
      appendNumber(dy, '$');
      x = -1;
      y = yi;
    }
    int dx = xi - x;
    if (dx <= 0)
      throw std::logic_error( "Cell list is not sorted by X" );
    if (dx == 1){
      count++; //continue current horizontal line
    }else if (dx > 1){ //line broken
      endWritingBlock();
      appendNumber(dx - 1, 'b');  //write whitespace before next block
      count = 1; //and remember the current cell
    }
    x = xi;
  }
  endWritingBlock();
  return rle.str();
}

void evaluateCellList(const MargolusBinaryRule &rule, const Pattern &cells, int phase, Pattern&transformed) {
  //var b_x, b_y, block, block2cells, key, transformed, x, x_code, y, y_code, _, _i, _len, _ref, _ref1, _ref2;
  using namespace std;

  if (rule(0) != 0) {
    throw logic_error("Rule has instable vacuum and not supported.");
  }
  map<std::tuple<int,int>, default_int> block2cells;
  for (const Cell& xy: cells.points) {
    int x = xy[0] + phase, y = xy[1] + phase;
    int mask = 1 << (mod2(x) + mod2(y)*2);
    block2cells[make_tuple(div2(x), div2(y))].value |= mask; //if value was not present, it will be initialized by 0.
  }

  transformed.points.clear();
  transformed.points.reserve( cells.size() );
  for (auto &iBlock : block2cells) {
    int x_code = iBlock.second.value;
    int b_x = std::get<0>(iBlock.first);
    int b_y = std::get<1>(iBlock.first);
    b_x = (b_x * 2) - phase;
    b_y = (b_y * 2) - phase;
    int y_code = rule(x_code);
    if (y_code & 1) 
      transformed.append(Cell(b_x, b_y));
    if (y_code & 2)
      transformed.append(Cell(b_x + 1, b_y));
    if (y_code & 4)
      transformed.append(Cell(b_x, b_y + 1));
    if (y_code & 8)
      transformed.append(Cell(b_x + 1, b_y + 1));
  }
}

bool isOffsetEqual( const Pattern &p1, const Pattern &p2, Cell &offset)
{
  if(p1.size() != p2.size()) return false;
  if(p1.size() == 0) {
    offset = Cell(0,0);
    return true;
  };
  offset = p2.points[0] - p1.points[0];
  int sz = p1.size();
  for( int i=1; i<sz; ++i){
    if (p2.points[i] - p1.points[i] != offset)
      return false;
  }
  return true;
}

bool checkOffsetOddity(const Cell &offset, bool mustBeOdd)
{
  return 
    (odd(offset[0]) == mustBeOdd) &&
    (odd(offset[1]) == mustBeOdd);
}
bool isOffsetEqualWithOddity( const Pattern &p1, const Pattern &p2, bool isOffsetOdd, Cell &offset)
{
  if(p1.size() != p2.size()) return false;
  if(p1.size() == 0) {
    offset = Cell(0,0);
    return true;
  };
  offset = p2.points[0] - p1.points[0];
  if (! checkOffsetOddity(offset, isOffsetOdd) ) return false; //wrong oddity
  int sz = p1.size();
  for( int i=1; i<sz; ++i){
    if (p2.points[i] - p1.points[i] != offset)
      return false;
  }
  return true;
}

void Pattern::from_rle( const std::string &rle_string )
{
  char c;
  int count, curCount=0, x=0, y=0;
  size_t nchars = rle_string.size();
  for (size_t i = 0; i < nchars; ++i) {
    c = rle_string[i];
    if (('0' <= c && c <= '9')) {
      curCount = curCount * 10 + (int(c)-int('0'));
    } else {
      count = (std::max)(curCount, 1);
      curCount = 0;
      switch (c) {
        case 'b':
          x += count;
          break;
        case '$':
          y += count;
          x = 0;
          break;
        case 'o':
          for (int j = 0; j < count; ++j) {
            append(x, y);
            x += 1;
          }
          break;
      default:{
	std::ostringstream msg;
	msg<<"Unexpected character in rle: "<<c<<" at position "<<i;
	throw std::logic_error(msg.str());
      }
      }
    }
  }
}

//affine transform the pattern, no normalization. might be self.
void Pattern::transform( const Transform &t, Pattern &to )const
{
  if (&to != this){
    to.points.clear();
    to.points.reserve(size());
  }
  for( const Cell &xy: points ){
    //the idea is:
    // transform so that the first block: (0,0), (0,1), (1,0), (1,1) maps to itself
    // to do this, we need: tfm( x- 0.5)+0.5
    // to avoid floating-point operations, scale everything by 2
    int x = xy[0]*2-1;
    int y = xy[1]*2-1;
    int x1 = div2(t.tx(x,y)+1);
    int y1 = div2(t.ty(x,y)+1);
    if (&to != this)
      to.append( x1, y1 );
    else
      //Transform inplace
      const_cast<Cell&>(xy) = Cell(x1,y1);
  }
}


bool Pattern::operator == (const Pattern &p)const
{
  return p.points == points;
}
