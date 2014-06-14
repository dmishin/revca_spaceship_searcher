#ifndef __MAHTUTIL_HPP_INCL__
#define __MAHTUTIL_HPP_INCL__
inline int div2(int x){ return x >> 1;};
inline int mod2(int x){ return x &  1;};

inline int mod(int x, int y)
{
  int m =  x % y;
  return (m<0)?(m+y):m;
}


inline int mdiv(int x, int y)
{
  if (y < 0) return mdiv(-x,-y);
  if (x>=0)
    return x/y;
  else
    return (x-mod(x,y))/y;
};

inline bool odd(int x)
{
  return (x % 2) != 0;
};

template <class T, class MeasureFunction, class Measure=double>
struct Maximizer{
  bool hasValue;
  Measure bestMeasure;
  T bestValue;
  MeasureFunction measureFunc;
  explicit Maximizer( MeasureFunction func=MeasureFunction() )
    :hasValue(false)
    ,measureFunc(func)
  {};

  void put( const T& value){
    Measure m = measureFunc(value);
    if ((!hasValue) || (m > bestMeasure)){
      bestMeasure = m;
      bestValue = value;
    }
    hasValue = true;
  };
  const T& getBestValue()const{
    if (!hasValue)  throw std::logic_error("No values to choose from");
    return bestValue;
  }
};

/**default-constructible int*/
struct default_int{
  int value;
  default_int( int v ): value(v){};
  default_int(): value(0){};
  default_int( const default_int &di ): value(di.value){};
  default_int& operator=(const default_int &di){ value=di.value; return *this; };
  bool operator == (const default_int &i)const{ return value==i.value; };
  bool operator != (const default_int &i)const{ return !(*this==i);};
};

#endif
