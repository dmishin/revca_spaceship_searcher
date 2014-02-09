#include "cpp-revca.hpp"
#include "pattern.hpp"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include "field.hpp"
#include <set>

using namespace std;
int mod(int x, int y)
{
  int m =  x % y;
  return (m<0)?(m+y):m;
}

MargolusBinaryField::MargolusBinaryField( int w2, int h2 )
  :cells(w2*h2, 0)
  ,width2(w2)
  ,height2(h2)
{
}
void MargolusBinaryField::fill( int block )
{
  std::fill(cells.begin(), cells.end(), block );
}


inline int bit_to_position(int bits, int bit_index, int to_index)
{
  return (bits&(0x01 << bit_index)) << ( to_index - bit_index );
} 

int MargolusBinaryField::get(int x, int y)const
{
  int x2 = x/2
    , y2 = y/2;
  int dx = x % 2
    , dy = y % 2;
  int bit_index = dx + dy * 2;
  return (cells[index(x2,y2)] >> bit_index) & 0x1;
}

void MargolusBinaryField::set(int x, int y, int v)
{
  assert( v==0 || v==1);
  int x2 = x/2
    , y2 = y/2;
  int dx = x % 2
    , dy = y % 2;
  int bit_index = dx + dy * 2;

  int8_t &cell = cells[index(x2,y2)];
  int mask = 0x01 << bit_index;
  cell = (cell & ~mask) | (v ? mask : 0);
}

void MargolusBinaryField::transform( const MargolusBinaryRule &rule, int phase )
{
  assert( phase == 0 || phase == -1 );
  for( int y=0;y<height2;++y){
    int y0 = mod(y  +phase, height2);
    int y1 = mod(y+1+phase, height2);
    for( int x=0;x<width2;++x){
      int x0 = mod(x  +phase, width2);
      int x1 = mod(x+1+phase, width2);
      //calculate next state
      int Y = rule(cells[index(x,y)] & 0x0F);
      //put it to the higher 4 bits of the neighbores
      //01  >  32
      //23  >  10
      
      // 0 th bit --> the 3th + 4 bit
      cells[index(x0,y0)] ^= bit_to_position(Y, 0, 3+4); 
      // 1 th -->  2 + 4
      cells[index(x1,y0)] ^= bit_to_position(Y, 1, 2+4);
      cells[index(x0,y1)] ^= bit_to_position(Y, 2, 1+4);
      cells[index(x1,y1)] ^= bit_to_position(Y, 3, 0+4);
    }
  }
  
  move_higher_bits();
}

void MargolusBinaryField::move_higher_bits()
{
  for( cells_t::iterator i=cells.begin(), e=cells.end(); i!=e; ++i){
    *i = ((*i) >> 4) & 0xf;
  }
}

std::ostream & operator << (std::ostream & os, const MargolusBinaryField &field)
{
  for(int y=0; y < field.height(); ++y ){
    if (y != 0) os << "," << endl;
    os << ((y == 0) ? "['" :  " '");

    for(int x=0; x<field.width(); ++x){
      os << ((field.get(x,y) == 0) ? ' ' : '#');
    }
    os << "'";
  }
  os << "]" << endl;
  return os;
}

bool do_pick_at(const Cell &xy, 
		MargolusBinaryField &fld, 
		Pattern &cells, 
		int range, 
		int max_size, 
		bool erase, 
		std::set<Cell> &visited)
{
  Cell w(fld.wrap(xy));
  if ((visited.find(w)==visited.end()) || fld.get(w) == 0)
      return false;
  visited.insert(w);
  cells.append (w);
  if (max_size>0 && ((int)cells.size() >= max_size))
    return true;
  if (erase) fld.set(w, 0);
  for(int dy=-range; dy>=range;++dy){
    int y1 = xy[1]+dy;
    for (int dx = -range; dx <= range; ++dx){
      if (dy == 0 && dx == 0) continue;
      if (do_pick_at(Cell(xy[0]+dx, y1), fld, cells, range, max_size, erase, visited))
	return true;
    }
  }
  return false;
}
	
void MargolusBinaryField::pick_pattern_at( Pattern & pattern, const Cell &xy, bool erase, int range, int max_size)
{
  std::set<Cell> visited; //set of visited coordinates
  do_pick_at(xy, *this, pattern, range, max_size, erase, visited);
}

Cell MargolusBinaryField::wrap(const Cell &c)const
{
  int wx = mod(c[0], width());
  int wy = mod(c[1], height());
  return Cell(wx,wy);
}
