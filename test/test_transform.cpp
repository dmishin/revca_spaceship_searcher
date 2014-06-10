#include "gtest/gtest.h"
#include "pattern.hpp"

TEST( Transform, construction ){
  Transform eye(1,2,3,4);
  EXPECT_EQ( eye.matrix[0], 1 );
  EXPECT_EQ( eye.matrix[1], 2 );
  EXPECT_EQ( eye.matrix[2], 3 );
  EXPECT_EQ( eye.matrix[3], 4 );
}

TEST( Transform, equality ){
  Transform t1(1,2,3,4);
  Transform t2(2,3,4,5);
  EXPECT_EQ( t1, t1);
  EXPECT_EQ( t2, t2);
  EXPECT_NE( t1,t2);
  EXPECT_NE( t2, t1);
}
TEST( Transform, assign ){
  Transform t1(1,2,3,4);
  Transform t2(2,3,4,5);
  EXPECT_NE( t1,t2);
  t1 = t2;
  EXPECT_EQ( t1, t2);
}

TEST( Transform, tfm ){
  Transform eye(1,0,0,1);
  EXPECT_EQ( eye(Cell(0,0)), Cell(0,0) );
  EXPECT_EQ( eye(Cell(0,10)), Cell(0,10) );
  EXPECT_EQ( eye(Cell(-10,10)), Cell(-10,10) );

  Transform flip(0,1,1,0);
  EXPECT_EQ( flip(Cell(0,0)), Cell(0,0) );
  EXPECT_EQ( flip(Cell(0,10)), Cell(10,0) );
  EXPECT_EQ( flip(Cell(-10,10)), Cell(10,-10) );

  Transform rot(0,-1,1,0);
  EXPECT_EQ( rot(Cell(0,0)), Cell(0,0) );
  EXPECT_EQ( rot(Cell(0,10)), Cell(-10,0) );
  EXPECT_EQ( rot(Cell(-10,0)), Cell(0,-10) );
  EXPECT_EQ( rot(Cell(0,-10)), Cell(10,0) );
}

TEST( Transform, multiply ){
  Transform eye(1,0,0,1);
  Transform flip(0,1,1,0);
  Transform rot(0,-1,1,0);


  EXPECT_EQ( eye, eye*eye );
  EXPECT_EQ( eye, flip*flip );
  EXPECT_NE( eye, rot*rot );
  EXPECT_NE( eye, rot*rot*rot );
  EXPECT_EQ( eye, rot*rot*rot*rot );
  EXPECT_EQ( flip, eye*flip);
  EXPECT_EQ( flip, flip*eye);
  EXPECT_EQ( rot, eye*rot);
  EXPECT_EQ( rot, rot*eye);
}
