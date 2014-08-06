#ifndef __THREE_PATTERN_HPP_INCLUDED__
#define __THREE_PATTERN_HPP_INCLUDED__
#include "pattern.hpp"
#include "mathutil.hpp"

#include <map>
#include <iostream>

class MargolusBinaryRule;
class Transform;

/**Alternative implementation of the pattern*/
class TreePattern{
public:
  std::map<Cell, default_int> blocks;

  TreePattern()
  {};
  TreePattern( const TreePattern &p )
    :blocks(p.blocks)
  {}
  TreePattern( const Pattern& p);
  TreePattern( std::string &rle);
  TreePattern( std::initializer_list<Cell> cells );
  
  TreePattern & append( int x, int y);
  TreePattern & append( const Cell &c){ return append(c[0],c[1]); };

  void swap(TreePattern &p);
  void clear(){ blocks.clear(); };
  
  bool operator == (const TreePattern &p)const;
  bool operator != (const TreePattern &p)const { return !(*this == p); }

  TreePattern &operator =(const TreePattern &p){ blocks=p.blocks; return *this; };
  
  void to_list( Pattern & to )const;
  Pattern to_list()const{ Pattern p; to_list(p); return p;};
  void from_list( const Pattern &lst );
  
  
  std::string to_rle()const;
  void from_rle( const std::string &rle);

  //geometry info
  Cell top_left_even()const;
  //  Cell bottom_right_even()const;

  void translate_even(int dx2, int dy2, TreePattern&to)const;
  void translate_even(int dx2, int dy2);
  void translate(int dx, int dy, TreePattern &to)const;
  void translate(int dx, int dy);


  void transform( const Transform &tfm, TreePattern &to)const;


  void evaluate( const MargolusBinaryRule &rule, int phase, TreePattern &out);
  
  bool shift_equal( const TreePattern &p, Cell &shift)const;

  friend std::ostream & operator << (std::ostream &os, const TreePattern &p);

  bool at( int x, int y )const;

  size_t blocks_size()const{ return blocks.size(); };
  size_t size()const;
  std::pair<Cell, Cell> block_bounds()const;
  bool empty()const{ return blocks.empty(); };
};

std::ostream & operator << (std::ostream &os, const TreePattern &p);

#endif
