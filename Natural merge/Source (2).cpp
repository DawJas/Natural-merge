#include "natural_merge.hpp"
/* 
Dawid Jastrzebski s188733
Struktury Baz Danych 
04.11.2023

Sorting sequential files using natural merge 2+1 
Record type: Sets of letters with max lenght of 30.
*/

int main() {
	input();

	while(1) {
		split();
		if(merge()) break;
	}

	display_stats();

	return 0;
} 
