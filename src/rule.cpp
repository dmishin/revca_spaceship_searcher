#include "cpp-revca.hpp"
#include "pattern.hpp"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <set>

#include "random_search.hpp"
#include "field.hpp"

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
  ios::fmtflags f( os.flags() );
  os<<"[";
  for(int x=0; x<16; ++x){
    if (x != 0) os << " ";
    os << hex << x ;
    os << "->";
    os << hex << rule(x) ;

  }
  os.flags( f );
  return os;
}


