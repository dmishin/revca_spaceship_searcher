#include "tree_pattern.hpp"
#include "mathutil.hpp"
#include "rule.hpp"

#include <iostream>
#include <stdexcept>
//#include <algorithm>

/**Alternative implementation of the pattern*/
TreePattern::TreePattern( const Pattern& p)
{
  from_list(p);
}

TreePattern::TreePattern( std::string &rle)
{
  from_rle(rle);
}

TreePattern::TreePattern( std::initializer_list<Cell> cells )
{
  for( auto &cell : cells)
    append(cell);
}


inline int mask( int bx, int by )
{
  return 1 << (bx + by*2);
}
  
TreePattern & TreePattern::append( int x, int y)
{
  blocks[Cell(div2(x), div2(y))].value |= mask(mod2(x), mod2(y));
  return *this;
}

bool TreePattern::operator == (const TreePattern &p)const
{
  return blocks == p.blocks;
}

template< class Func >
void for_each_cell( const std::map<Cell, default_int> blocks, Func f)
{
  for( auto &item: blocks ){
    int b_x = item.first[0]*2;
    int b_y = item.first[1]*2;
    int y_code = item.second.value;
    if (y_code & mask(0,0)) 
      f(b_x, b_y);
    if (y_code & mask(1,0))
      f(b_x + 1, b_y);
    if (y_code & mask(0,1))
      f(b_x, b_y + 1);
    if (y_code & mask(1,1))
      f(b_x + 1, b_y + 1);
  }
}
  
void TreePattern::to_list( Pattern & to )const
{
  for_each_cell(blocks, [ &to ](int x, int y){ 
      to.append(x,y); 
    });
}

std::string TreePattern::to_rle()const
{
  Pattern p;
  to_list(p);
  p.sort();
  return p.to_rle();
}

void TreePattern::from_rle( const std::string &rle)
{
  Pattern p;
  p.from_rle(rle);
  from_list(p);
}
void TreePattern::from_list( const Pattern &lst )
{
  for( const Cell & cell: lst.points )
    append( cell );
}

void TreePattern::evaluate( const MargolusBinaryRule &rule, int phase, TreePattern &out)
{
  if (rule(0) != 0) throw std::logic_error("vacuum-instable rules are not supported");
  for( auto &item: blocks ){
    int b_x = item.first[0]*2-2*phase+1;
    int b_y = item.first[1]*2-2*phase+1;
    int y_code = rule(item.second.value);
    // phase 0:
    // 0,0 -> 1,1
    // 0,1 -> 1,2
    // ...

    //phase 1:
    // 0,0 -> -1, -1

    if (y_code & mask(0,0)) 
      out.append(b_x, b_y);
    if (y_code & mask(1,0))
      out.append(b_x + 1, b_y);
    if (y_code & mask(0,1))
      out.append(b_x, b_y + 1);
    if (y_code & mask(1,1))
      out.append(b_x + 1, b_y + 1);
  }
}
/*
template <class FoldFunc, class RetVal>
RetVal fold_blocks( const std::map<Cell, default_int> & blocks, RetVal initial, FoldFunc fold)
{
  RetVal x(initial);
  for( auto iblock: blocks ){
    x = FoldFunc( x, *iblock );
  }
  
}
*/
Cell TreePattern::top_left_even()const
{
  int xmin=0, ymin=0;
  bool first=true;
  for( auto &iblock: blocks ){
    const Cell &xy = iblock.first;
    if (first) {
      xmin = xy[0]; ymin=xy[1];
      first = false;
    }else{
      xmin = (std::min)( xmin, xy[0]);
      ymin = (std::min)( ymin, xy[1]);
    }
  }
  return Cell(xmin, ymin);
}
void TreePattern::translate_even(int dx2, int dy2)
{
  TreePattern translated;
  Cell shift(dx2,dy2);
  for( auto &iblock: blocks ){
    translated.blocks[iblock.first+shift]=iblock.second;
  }
  swap(translated);
}

void TreePattern::translate(int dx, int dy)
{
  if ((mod2(dx)==0) && (mod2(dy)==0)){
    translate_even(dx/2,dy/2);
  }else{
    TreePattern translated;
    for_each_cell(blocks, [&translated,dx,dy](int x, int y){
	translated.append(x+dx,y+dy);
      });
    swap(translated);
  }
}

void TreePattern::swap(TreePattern &p)
{
  blocks.swap(p.blocks);
}


bool TreePattern::shift_equal( const TreePattern &p, Cell &shift_)
{
  if (blocks.size() != p.blocks.size())
    return false;

  auto itr1 = blocks.begin();
  auto itr2 = p.blocks.begin();
  Cell shift(0,0);
  bool first=true;
  while( itr1 != blocks.end() && itr2 != p.blocks.end() ){
    if (itr1->second != itr2->second) return false;
    if (first){
      first = false;
      shift = itr2->first - itr1->first;
    }else{
      if (shift != itr2->first - itr1->first)
	return false;
    }
    ++itr1; ++itr2;
  }
  shift_ = shift;
  return true;
}

bool TreePattern::at( int x, int y )const
{
  int bx = div2(x), by = div2(y);
  auto iblock = blocks.find( Cell(bx, by ) );
  if (iblock == blocks.end()) return false;
  return (iblock->second.value & mask(mod2(x), mod2(y))) != 0;
}

std::ostream & operator << (std::ostream &os, const TreePattern &p)
{
  os<<"[";
  bool first = true;
  for_each_cell(p.blocks, [&os, &first](int x, int y){
      if (first)
	first = false;
      else
	os<<',';
      os<<'['<<x<<','<<y<<']';
    });
  return os<<']';
}

