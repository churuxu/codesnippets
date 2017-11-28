#include "HashExpr.hpp"
#include <stdio.h>


void simple_switch(char const* str) {
	using namespace std;
	switch (HASH(str)) {
	case HASH("first"):
		printf("input is first\n");
		break;
	case HASH("second"):
		printf("input is second\n");
		break;
	case HASH("third"):
		printf("input is third\n");
		break;
	default:
		printf("input not match\n");
	}
}


int main(int argc, char* argv[]) {	
	simple_switch("second");
	simple_switch("second2");
	simple_switch("third");
	if (argc > 1) {
		simple_switch(argv[1]);
	}
	return 0;
}