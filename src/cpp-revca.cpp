#include <cpp-revca.hpp>
#include <iostream>
#include <algorithm>
#include <stdexcept>

using namespace std;

void MargolusBinaryRule::set(int index, int x)
{
  assert(index >=0 && index <16);
  assert(x>=0 && x <16);
  table[index] = x;
}

MargolusBinaryRule::MargolusBinaryRule(int (&values)[16]  )
{
  copy( & values[0], (& values[0]) + 16, &table[0] );
}


ostream & operator << (ostream & os, const MargolusBinaryRule &rule)
{
  os<<"[";
  for(int x=0; x<16; ++x){
    if (x != 0) os << " ";
    os << hex << x ;
    os << "->";
    os << hex << rule(x) ;

  }
  return os;
}

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

int main(int argc, char* argv[])
{
  int r[] = {0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15};
  MargolusBinaryRule rule(r);

  cout << rule << endl;

  MargolusBinaryField fld(5,5);
  fld.set(2,2,1);
  fld.set(2,3,1);
  fld.set(2,4,1);
  fld.set(4,3,1);

  cout << "Before transform:"<<endl;
  cout << fld;


  fld.transform( rule, 0 );
  cout << "After transform 1:"<<endl;
  cout << fld;


  fld.transform( rule, -1 );
  cout << "After transform 2:"<<endl;
  cout << fld;
  return 0;
}
