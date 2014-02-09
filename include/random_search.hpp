#ifndef _RANDOM_SEARCH_HPP_INCLUDED_
#define _RANDOM_SEARCH_HPP_INCLUDED_
class MargolusBinaryField;
class MargolusBinaryRule;
class Cell;

void random_fill( MargolusBinaryField &fld, const Cell& p0, const Cell& p1, double percent);
void do_random_search( const MargolusBinaryRule &rule,
		       int field_size, int random_box_size,
		       double random_percent, 
		       int generations,
		       int pick_range, 
		       int max_pattern_size );

#endif
