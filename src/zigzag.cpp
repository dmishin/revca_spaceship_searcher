#include "zigzag.hpp"

/*
def _cone_increase_at(x,i):
    v = x[i] + 1
    if (i < len(x)-1) and (x[i+1] < v):
        x[i] = 0
        _cone_increase_at(x,i+1)
    else:
        x[i] = v
*/
void _cone_increase_at(int *x, size_t sz, size_t i)
{
  int v = x[i] + 1;
  if ((i < sz-1) && (x[i+1] < v)){
    x[i] = 0;
    _cone_increase_at(x,sz, i+1);
  }else{
    x[i] = v;
  }
}
/*
def cone_next( x ):
    """walk in the space of increasing n-vectors {x_i}, having the property: x_{i+1} >= x_i
    0-1-2-3-4-...
    00-01-11-02-12-22-03...
    000-001-011-111-002-012...
    """
    return _cone_increase_at(x, 0)
*/

void cone_next( int*x, size_t sz)
{
  _cone_increase_at(x, sz, 0);
}
