#ifndef __PATTERN_HPP_INCLUDED__
#define __PATTERN_HPP_INCLUDED__
#include <tuple>
#include <vector>
#include <iostream>
#include <cassert>
#include <string>

class MargolusBinaryField;

struct Cell
{
  int coords[2];
  int operator[](int idx)const{ assert(idx >=0 && idx <2); return coords[idx]; };
  int& operator[](int idx){ assert(idx >=0 && idx <2); return coords[idx]; };
  Cell( int x, int y ): coords{x,y}{};
  Cell(){};
  bool operator == (const Cell &c)const{ return (*this)[0]==c[0] && (*this)[1]==c[1]; };
  bool operator != (const Cell &c)const{ return ! (*this == c); };
  bool operator < (const Cell &c)const{
    if ((*this)[1] < c[1]) return true;
    if ((*this)[1] > c[1]) return false;
    return (*this)[0] < c[0];
  }
  Cell & operator += (const Cell &c){
    (*this)[0] += c[0];
    (*this)[1] += c[1];
    return *this;
  };
  Cell & operator -= (const Cell &c){
    (*this)[0] -= c[0];
    (*this)[1] -= c[1];
    return *this;
  };
  Cell operator + (const Cell &c)const{
    return Cell(*this) += c;
  };
  Cell operator -()const{
    return Cell(-coords[0], -coords[1]);
  };
  Cell operator - (const Cell &c)const{
    return Cell(*this) -= c;
  };
};

inline std::ostream & operator << (std::ostream &os, const Cell &c)
{
  return os<<"["<<c[0]<<","<<c[1]<<"]";
}

class Transform
{
public:
  int matrix[4];
  
  Transform( int a00=1, int a01=0, int a10=0, int a11=1)
    :matrix{a00,a01,a10,a11}{};
  int tx( int x, int y )const{ 
    return matrix[0]*x + matrix[1]*y;
  };
  int ty( int x, int y )const{ 
    return matrix[2]*x + matrix[3]*y;
  };
  Cell operator()( const Cell & c )const{
    return Cell( tx(c[0],c[1]), ty(c[0],c[1]) );
  }
  void set( int a00=1, int a01=0, int a10=0, int a11=1 )
  {
    matrix[0]=a00; matrix[1]=a01; matrix[2]=a10; matrix[3]=a11;
  };
  Transform operator*(const Transform &t){
    return  Transform(tx(t.matrix[0], t.matrix[2]), tx(t.matrix[1], t.matrix[3]),
		      ty(t.matrix[0], t.matrix[2]), ty(t.matrix[1], t.matrix[3]));
  };
  bool operator == (const Transform &t)const{
    for(int i=0; i<4; ++i)
      if (matrix[i]!=t.matrix[i]) return false;
    return true;
  };
  bool operator != (const Transform &t)const{ return ! ((*this)==t); };
  
};

class Pattern
{
public:
  typedef std::vector< Cell > points_t;
private:
  std::tuple<int, int> coord_range(int coord_index)const;
public:
  points_t points;

  Pattern(){};
  Pattern( const Pattern &p ): points(p.points) {};

  Pattern & append( const Cell &c){points.push_back(c);return *this;};
  Pattern & append( int x, int y);

  void sort();
  void translate( int dx, int dy ){translate(Cell(dx,dy));};
  void translate( const Cell & to );

  std::tuple<Cell, Cell> bounds()const;
  Cell top_left()const;
  Cell top_left_even()const;
  void normalize();

  void put_to(MargolusBinaryField &fld, int x0, int y0, const Transform &tfm )const;
  size_t size()const{ return points.size(); };
  //Convert list of alive cells to RLE. List of cells must be sorted by Y, then by X, and coordinates of origin must be at (0,0)
  std::string to_rle()const;
  void from_rle( const std::string &rle );
};

std::ostream & operator <<(std::ostream &os, const Pattern &p);

class MargolusBinaryRule;
void evaluateCellList(const MargolusBinaryRule &rule, const Pattern &cells, int phase, Pattern&transformed);

//true, if two patterns are equal due to some offset.
//returns the offset, if true.
bool isOffsetEqual( const Pattern &p1, const Pattern &p2, Cell &offset);
bool isOffsetEqualWithOddity( const Pattern &p1, const Pattern &p2, bool isOffsetOdd, Cell &offset);
bool odd(int x);

#endif


