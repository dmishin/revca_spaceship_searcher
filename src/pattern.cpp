#include "pattern.hpp"
#include <algorithm>
#include "cpp-revca.hpp"
#include "field.hpp"
#include <sstream>
#include <map>
#include <tuple>
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
  if (rule(0) != 0) {
    throw std::logic_error("Rule has instable vacuum and not supported.");
  }
  std::map<std::tuple<int,int>, int> block2cells;
  for (const Cell& xy: cells.points) {
    int x = xy[0], y = xy[1];
    x += phase;
    y += phase;
    int b_x = x / 2;
    int b_y = y / 2;
    auto key = std::make_tuple(b_x, b_y);
    auto iBlock = block2cells.find(key);
    int mask = 1 << (mod(x , 1) + mod(y, 1) * 2);
    if (iBlock == block2cells.end()){
      block2cells[key] = mask;
    }else{
      iBlock->second |= mask;
    }
  }

  for (auto &iBlock : block2cells) {
    int x_code = iBlock.second;
    int b_x = std::get<0>(iBlock.first);
    int b_y = std::get<1>(iBlock.first);
    b_x = (b_x * 2) - phase;
    b_y = (b_y * 2) - phase;
    int y_code = rule(x_code);
    if (y_code & 1) {
      transformed.append(Cell(b_x, b_y));
    }
    if (y_code & 2) {
      transformed.append(Cell(b_x + 1, b_y));
    }
    if (y_code & 4) {
      transformed.append(Cell(b_x, b_y + 1));
    }
    if (y_code & 8) {
      transformed.append(Cell(b_x + 1, b_y + 1));
    }
  }
}
