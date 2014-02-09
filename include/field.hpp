#ifndef _FIELD_HPP_INCLDUED_
#define _FIELD_HPP_INCLDUED_
#include <vector>
#include "pattern.hpp"

class MargolusBinaryRule;

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
  int get(const Cell& c)const{ return get(c[0],c[1]); };
  void set(int x, int y, int v);
  void set(const Cell& c, int v){ set(c[0],c[1],v); };
  void transform( const MargolusBinaryRule &rule, int phase );
  void transform2(const MargolusBinaryRule &rule){ transform(rule,0); transform(rule,-1); };

  void pick_pattern_at( Pattern & pattern, const Cell &xy, bool erase=false, int range=4, int max_size=-1);

  Cell wrap(const Cell &c)const;
protected:
  void move_higher_bits();
};

std::ostream & operator << (std::ostream & os, const MargolusBinaryField &field);

int mod(int x, int y);

#endif
