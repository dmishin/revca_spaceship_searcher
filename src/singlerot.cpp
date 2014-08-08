#include "singlerot.hpp"
#include "tree_pattern.hpp"
#include <vector>
using namespace std;

bool singlerot_has_unchanged_core( const Pattern & _p )
{
  TreePattern pattern(_p);
  while (! pattern.empty() ){
    vector<Cell> keys_to_delete;
    for( auto &key_value : pattern.blocks ){
      int block = key_value.second.value;
      if (block == 1 || block == 2 || block == 4 || block == 8){ //single-cellers
	keys_to_delete.push_back( key_value.first );
      }
    }
    if (keys_to_delete.empty())
      break;
    for( const Cell &key : keys_to_delete ){
      pattern.blocks.erase( key );
    }
    pattern.translate(1,1);
  }
  
  return ! pattern.empty();
}

bool SinglerotCoralFilter::check( const Pattern &p)
{ 
  return ! singlerot_has_unchanged_core(p);
};

MargolusBinaryRule singlerot({0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15});
