#include "pattern_enumerator.hpp"
#include "zigzag.hpp"
#include <cmath>

/**return s (x,y) of the point in the spiral coordinates. Point order is:
       ghijkl
        4567m
        f018n
        e329o
        dcbap
         ...q
*/
void spiral_point( int i, int &x, int &y )
{
  //start indices of the circles:
  // circle index: length: start index
  // 0:  4:  0
  // 1:  12: 4
  // 2:  20: 16(g)
  // .....
  // n:  4+8n: 4n+(n^2-n)/2*8=n^2*4
  
  //so, to determine point position
  //circle index
  int ci = (int)floor(sqrt(i)/2);
  int cstart = ci*ci*4;

  int j = i-cstart; //index inside the circle
  //int side = 2+ci*2; //circle side length
    
  //determine, which side the point is on, and then return the coordinates
  int x0, y0;
  if (j < ci*2+1){
    //first side
    x0 = -ci;
    y0 = -ci;
    x = x0+j; 
    y= y0;
  }else if ( j < ci*4+2){
    x0 = ci+1; // = -ci + 2*ci+1
    y0 = -ci;
    x =x0; 
    y= y0 + j - ci*2-1;
  }else if ( j < ci*6+3){
    x0 = ci+1;
    y0 = ci+1;
    x = x0 - (j-ci*4-2);
    y = y0;
  }else{
    //assert j < ci*8+4
    x0 = -ci;
    y0 = ci+1;
    x = x0; 
    y = y0- (j-ci*6-3);
  }
}


pattern_enumerator:: pattern_enumerator(int n)
  :state(n-1, 0)
{
}
void pattern_enumerator::next()
{
  cone_next(&(state[0]), state.size());
}
 

pattern_enumerator::pattern_enumerator(const std::vector<int> &index)
  :state(index)
{
}
