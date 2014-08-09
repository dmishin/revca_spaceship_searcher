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

MargolusBinaryRule::MargolusBinaryRule(const int (&values)[16]  )
{
  copy( & values[0], (& values[0]) + 16, &table[0] );
}


ostream & operator << (ostream & os, const MargolusBinaryRule &rule)
{
  ios::fmtflags f( os.flags() );
  os<<"[";
  for(int x=0; x<16; ++x){
    if (x != 0) os << ',';
    os << rule(x);
  }
  os<<"]";
  os.flags( f );
  return os;
}



bool MargolusBinaryRule::operator==(const MargolusBinaryRule &r)const
{
  for(int i=0; i<16; ++i)
    if (table[i] != r.table[i]) return false;
  return true;
}
