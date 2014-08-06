#include <iostream>
#include "pattern_enumerator.hpp"
#include "tree_pattern.hpp"
#include "analyze.hpp"


using namespace std;



int main( int argc, char *argv[] )
{

  pattern_enumerator enumer(4);
  TreePattern total_p;

  for(int i=0; i<300; ++i){
    cout<<"============"<<i<<"============="<<endl;

    TreePattern p;
    enumer.for_pattern( [&p, &total_p, i](int idx, int x, int y){
	p.append(x,y);
	total_p.append( x+(i%17)*8, y+(i/17)*8 );
      } );
    Cell tl = p.top_left_even();
    p.translate_even(-tl[0],-tl[1]);
      
    cout << p.to_rle() << endl;
    enumer.next();

  }
  Cell tl = total_p.top_left_even();
  total_p.translate_even(-tl[0],-tl[1]);
  cout << "Total pattern:"<<endl
       << total_p.to_rle() << endl;
    
  return 0;
}

