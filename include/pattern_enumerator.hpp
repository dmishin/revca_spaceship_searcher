#ifndef __PATTERN_ENUMERATOR_INCLUDED__
#define __PATTERN_ENUMERATOR_INCLUDED__

#include<vector>

void spiral_point( int i, int &x, int &y );

struct pattern_enumerator{
  std::vector<int> state;
  pattern_enumerator(int n);
  pattern_enumerator(const std::vector<int> &index);
  int patter_size()const{return state.size()+1; };
  void next();
  template< class CallBackT >
  void for_pattern( CallBackT add_cell_cb )const;  
};

template< class CallBackT >
void pattern_enumerator::for_pattern( CallBackT add_cell_cb )const
{
  int idx = 0;
  add_cell_cb(idx,0,0);
  idx ++;
  for(int i=0; i<(int)state.size();++i){
    int x,y;
    spiral_point(state[i]+i+1, x, y);
    add_cell_cb( i+1, x, y );
  }
};

#endif
