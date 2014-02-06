#ifndef __CPP_REVCA_HPP_INCLUDED__
#define __CPP_REVCA_HPP_INCLUDED__

#include <iostream>
#include <cstdint>
//typedef char int8_t;

#include <cassert>
#include <vector>



class MargolusBinaryRule{
  int table[16];
public:
  MargolusBinaryRule(){};
  MargolusBinaryRule(int (&values)[16]  );
  //MargolusBinaryRule( const MargolusBinaryRule& r );
  int operator()(int x)const{ return table[x]; };
  void set(int index, int x);

  
};

std::ostream & operator << (std::ostream & os, const MargolusBinaryRule &rule);


class MargolusBinaryField{
public:
  typedef std::vector<int8_t> cells_t;
private:
  cells_t cells;
  int width2, height2;
  
  size_t index( int x, int y )const{ return x+y*width2; };
public:
  MargolusBinaryField( int w2, int h2 );
  void fill( int block = 0 );
  int width()const{ return width2*2; };
  int height()const{ return height2*2; };

  int get(int x, int y)const;
  void set(int x, int y, int v);
  void transform( const MargolusBinaryRule &rule, int phase );
  void transform2(const MargolusBinaryRule &rule){ transform(rule,0); transform(rule,1); };
protected:
  void move_higher_bits();
};

std::ostream & operator << (std::ostream & os, const MargolusBinaryField &field);

int mod(int x, int y);


#endif
