#include "include/Vector2.h"

char * Vector2::toString() const{
	char str[2];

	str[0] = x;
	str[1] = y;

	return str;
}
