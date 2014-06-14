#include "gtest/gtest.h"
#include "tree_pattern.hpp"
#include "rule.hpp"
#include "analyze.hpp"

TEST( TreePattern, basic ){
  TreePattern p;  
  p.append( 1,1 );
  p.append( 2,2 );
  p.append( 3,3);

  TreePattern p1( p );

  EXPECT_TRUE( p == p );
  EXPECT_TRUE( p == p1 );

  EXPECT_FALSE( p != p );
  EXPECT_FALSE( p != p1 );

  TreePattern p2;
  p2.append(1,1).append(2,1).append(3,2);

  TreePattern p3;
  p3.append(1,1).append(1,2).append(2,3);
  
  EXPECT_EQ( p2,p2);
  EXPECT_NE( p1,p2);
  EXPECT_NE( p1,p3);

}
  

TEST(TreePattern, shifts){

  TreePattern p;  
  p.append( 1,1 ).append( 1,2 ).append( 4,3);
  
  TreePattern p1;
  p1.append( 2,2 ).append( 2,3 ).append( 5,4);
  
  //odd translation
  {
    TreePattern p_copy(p);
    p_copy.translate(1,1); 
    EXPECT_EQ( p1, p_copy );
  }

  //even translation

  TreePattern p2;
  p2.append( 1+2,1+4 ).append( 1+2,2+4 ).append( 4+2, 3+4);
  {
    TreePattern p_copy(p);
    p_copy.translate(2,4); 
    EXPECT_EQ( p2, p_copy );
  }  
}

TEST(TreePattern, at){
  TreePattern p{Cell(1,1),Cell(10,5),Cell(-1,-2),Cell(4,-4),Cell(3,4)};

  EXPECT_TRUE( p.at(1,1) );
  EXPECT_TRUE( p.at(10,5) );
  EXPECT_TRUE( p.at(-1,-2) );
  EXPECT_TRUE( p.at(4,-4) );
  EXPECT_TRUE( p.at(3,4) );

  EXPECT_FALSE( p.at(0,0) );
  EXPECT_FALSE( p.at(1,0) );
  EXPECT_FALSE( p.at(0,1) );
  EXPECT_FALSE( p.at(10,4) );
  EXPECT_FALSE( p.at(11,5) );
  EXPECT_FALSE( p.at(100,100) );
}

TEST(TreePattern, transform)
{
  MargolusBinaryRule single_rot({0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15}); //vivat c++11!

  TreePattern p{Cell(0,0)};

  TreePattern pt;
  p.evaluate(single_rot, 0, pt);
  
  //0,0 moves to 1.0; in the new tesselation, 1.0 goes to the block 1,0; bottom-left corner.

  EXPECT_EQ( TreePattern({Cell(2,1)}), pt );

  /////////////////////////////////////////
  // 3 more steps

  //second step
  p.swap(pt); 
  pt.clear();
  p.evaluate(single_rot, 1, pt);
  EXPECT_EQ( TreePattern({Cell(1,-1)}), pt );


  //third step
  p.swap(pt);
  pt.clear();
  p.evaluate(single_rot, 0, pt);

  //4'th step
  p.swap(pt);
  pt.clear();
  p.evaluate(single_rot, 1, pt);

  //now we should get the same as original
    
  EXPECT_EQ( TreePattern{Cell(0,0)}, pt );
}


TEST(TreePattern, analyze)
{
  MargolusBinaryRule single_rot({0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15});   

  TreePattern pat{Cell(0,0)}; //it's a period-4 oscillator

  AnalysysResult result = analyze_with_trees(pat, single_rot, 1000, 1000);

  EXPECT_EQ( AnalysysResult::CYCLE_FOUND, result.resolution );
  EXPECT_EQ( Cell(0,0), result.offset );
  EXPECT_EQ( 4, result.period );
  //not testing optimal form since it is not impleneted yet.
}
    
