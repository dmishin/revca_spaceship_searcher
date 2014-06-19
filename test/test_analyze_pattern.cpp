/*
describe "Cells.analyze() : analyze patterns", ->
  it "must detect block pattern correctly", ->
    pattern = Cells.from_rle "b2o$b2o"
    result = Cells.analyze pattern, single_rot
    assert result
    assert.equal result.dx, 0
    assert.equal result.dy, 0
    #visually, it is static, but phases are not equal.
    assert.equal result.period, 2

  it "must detect 1-cell pattern correctly", ->
    pattern = [[0,0]]
    result = Cells.analyze pattern, single_rot
    assert result
    assert.equal result.dx, 0
    assert.equal result.dy, 0
    #visually, it is static, but phases are not equal.
    assert.equal result.period, 4


  it "must detect light orthogonal spaceship correctly", ->
    pattern = Cells.from_rle "$2o2$2o"
    result = Cells.analyze pattern, single_rot
    assert result
    assert.equal result.dx, 2
    assert.equal result.dy, 0
    assert.equal result.period, 12


  it "must detect light diagonal spaceship correctly", ->
    pattern = Cells.from_rle "2bo$obo$o"
    result = Cells.analyze pattern, single_rot
    assert result
    assert.equal result.dx, 2
    assert.equal result.dy, 2
    assert.equal result.period, 48

  it "must detect long-period diagonal spaceship correctly", ->
    pattern = Cells.from_rle "o$o2$o$o"
    result = Cells.analyze pattern, single_rot
    assert result
    assert.equal result.dx, 2
    assert.equal result.dy, 2
    assert.equal result.period, 368
                        
  it "must successfully analyze some big spaceship", ->
    pattern = Cells.from_rle "b2obobo$4bo$4bo$4bo$6bo"
    result = Cells.analyze pattern, single_rot
    assert result
    assert.equal result.dx, 4
    assert.equal result.dy, 0
    assert.equal result.period, 242
      
  it "must rotate diagonal paceship to move in positive direction", ->
    r = Cells.analyze Cells.from_rle("$obo$b2o$2o"), single_rot
    assert.deepEqual [r.dx, r.dy], [1,1]

    r1 = Cells.analyze r.cells, single_rot
    assert.deepEqual [r1.dx, r1.dy], [1,1]

*/

#include "gtest/gtest.h"
#include "cpp-revca.hpp"
#include "pattern.hpp"
#include "analyze.hpp"

TEST( AnalyzeCellList, BlockPattern ){
    //testing serialization of the simple values
  using namespace std;
  MargolusBinaryRule single_rot({0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15});
  

  //it "must detect block pattern correctly", ->
  Pattern pattern;
  pattern.from_rle( "b2o$b2o" );
  AnalysysResult result;
  AnalyzeOpts opts;
  analyze(pattern, single_rot, opts, result);
  
  EXPECT_EQ( result.resolution, AnalysysResult::CYCLE_FOUND);
  EXPECT_EQ( result.offset, Cell(0,0) );
  //#visually, it is static, but phases are not equal
  EXPECT_EQ( result.period, 2);
  
  
  /*
  EXPECT_EQ( w.level(), (size_t)0 );
  */
}

TEST( AnalyzeCellList, SingleCell ){
  using namespace std;
  MargolusBinaryRule single_rot({0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15});
  
  //it "must detect 1-cell pattern correctly", ->
  Pattern pattern; pattern.append(0,0);
  AnalyzeOpts opts;
  AnalysysResult result;
  analyze( pattern, single_rot, opts, result );

  EXPECT_EQ (result.offset, Cell(0,0));
  //visually, it is static, but phases are not equal.
  EXPECT_EQ (result.period, 4 );
}

TEST( AnalyzeCellList, Spaceship ){
  using namespace std;
  MargolusBinaryRule single_rot({0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15});
  //it "must detect light orthogonal spaceship correctly", ->
  Pattern pattern; pattern.from_rle("$2o2$2o");
  AnalyzeOpts opts;
  AnalysysResult result;

  analyze( pattern, single_rot, opts, result );
  EXPECT_EQ( result.resolution, AnalysysResult::CYCLE_FOUND );
  EXPECT_EQ( Cell(2,0), result.offset );
  EXPECT_EQ( 12, result.period);
}
