#include "rule.hpp"
#include "cpp-revca.hpp"
#include "pattern.hpp"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <set>

#include "random_search.hpp"
#include "field.hpp"

using namespace std;

class TestPatternListener: public PatternListener
{
public:
  virtual void onPattern( const Pattern&p );
};
void TestPatternListener::onPattern(const Pattern&p)
{
  std::cout<<"Pattern found: "<<p.size()<<" cells "<<p.to_rle()<<std::endl;
  /*
  std::cout<<"  analyzing...";
      AnalyzeOpts opts={2048, 100, 1000};
      AnalysysResult result;
      analyze( *p, rule, opts, result);
      std::cout << result.resolution <<std::endl;
      if (result.period > 0){
	std::cout <<"  Period found:"<<result.period
		  <<" offset:"<<result.offset
		  <<" canonic RLE:" << result.bestPattern.to_rle()
		  << std::endl;
      }
  */
}
int main1(int argc, char* argv[])
{
  int r[] = {0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15};
  MargolusBinaryRule rule(r);

  cout << "Rule is: "<<rule << endl;

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

  TestPatternListener listener;
  do_random_search(rule, 128, 80, 0.4, 10000, 4, 20, listener);
  return 0;
  cout<< "Testing patterns"<<endl;
  Pattern p;
  p.append(1,2).append(2,3).append(3,4).append(5,5);
  cout << "Pattern:"<< p <<endl;
  p.sort();
  cout << " sorted:"<<p<<endl;
  //testing search

  return 0;
}




int main(int argc, char* argv[])
{
  int r[] = {0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15};
  MargolusBinaryRule rule(r);

  cout << "Rule is: "<<rule << endl;
  cout << "Testing patterns"<<endl;
  Pattern p;
  p.append(1,2).append(2,3).append(3,4).append(5,5);
  cout << "Pattern:"<< p << "=" << p.to_rle() <<endl;
  
  p.sort();
  cout << " sorted:"<<p<<endl;
  //testing evaluation

  
  Pattern t1;
  evaluateCellList(rule, p, 0, t1);
  p = t1;    
  t1.sort();
  cout << "After evaluation:"<< t1 << "=" << t1.to_rle() <<endl;

  evaluateCellList(rule, p, 1, t1);
  t1.sort();
  cout << "One more time:"<< t1<< "=" << t1.to_rle() <<endl;
  
  return 0;
}



