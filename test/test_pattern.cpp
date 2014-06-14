#include "gtest/gtest.h"
#include "pattern.hpp"
#include "analyze.hpp"

TEST( Pattern, Rle1 ){
  Pattern p;
  p.append( 0, 0);
  
  EXPECT_EQ( "o", p.to_rle() );

  p.append(1,1);
  EXPECT_EQ( "o$bo", p.to_rle() );

  p.append(2,2);
  EXPECT_EQ( "o$bo$2bo", p.to_rle() );
}

TEST( Pattern, Rle2 ){
  Pattern p;
  p.append( 2, 2);
  EXPECT_EQ( "2$2bo", p.to_rle() );
  
}


TEST( Pattern, Normalize1 ){
  Pattern p1;
  p1.append( 2, 2);
  Pattern p2;
  p2.append( 0,0 );

  p1.normalize();
  EXPECT_EQ( p2, p1 );
  
}

TEST( Pattern, Normalize2 ){
  Pattern p1;
  p1.append( 3, 1);
  Pattern p2;
  p2.append( 1, 1);

  p1.normalize();
  EXPECT_EQ( p2, p1 );
  
}
