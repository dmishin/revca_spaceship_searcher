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
  os<<"[";
  for(int x=0; x<16; ++x){
    if (x != 0) os << " ";
    os << hex << x ;
    os << "->";
    os << hex << rule(x) ;

  }
  return os;
}


int main(int argc, char* argv[])
{
  int r[] = {0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15};
  MargolusBinaryRule rule(r);

  cout << rule << endl;

  MargolusBinaryField fld(5,5);
  fld.set(2,2,1);
  fld.set(2,3,1);
  fld.set(2,4,1);
  fld.set(4,3,1);

  cout << "Before transform:"<<endl;
  cout << fld;


  fld.transform( rule, 0 );
  cout << "After transform 1:"<<endl;
  cout << fld;


  fld.transform( rule, -1 );
  cout << "After transform 2:"<<endl;
  cout << fld;

  for(int i=0; i<100; ++i)
    fld.transform2(rule);
  cout << "After transform 2+200:"<<endl;
  cout << fld;
  

  cout<< "Testing patterns"<<endl;
  Pattern p;
  p.append(1,2).append(2,3).append(3,4).append(5,5);
  cout << "Pattern:"<< p <<endl;
  p.sort();
  cout << " sorted:"<<p<<endl;
  //testing search
  do_random_search(rule, 32, 10, 0.3, 10000, 4, 100);
  return 0;
}



