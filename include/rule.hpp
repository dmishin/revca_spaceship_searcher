#ifndef __RULE_HPP_INCLUDED__
#define __RULE_HPP_INCLUDED__

#include <iostream>
#include <cstdint>
//typedef char int8_t;

#include <cassert>
#include <vector>
#include "pattern.hpp"

class MargolusBinaryRule{
  int table[16];
public:
  MargolusBinaryRule(){};
  MargolusBinaryRule(const int (& values)[16]  );

  //16 elements here
  //MargolusBinaryRule(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);
  
  //MargolusBinaryRule( const MargolusBinaryRule& r );
  int operator()(int x)const{ return table[x]; };
  void set(int index, int x);

  bool operator==(const MargolusBinaryRule &r)const;
  bool operator!=(const MargolusBinaryRule &r)const {return !(*this==r);};
  
  
  
};

std::ostream & operator << (std::ostream & os, const MargolusBinaryRule &rule);


#endif
