#pragma once
#pragma once

#include <stdlib.h>

#define SQU(X) (X) * (X)

/*
	random - Return a random number in the range of 0 - 1
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016

	return float - Returns the random number as a float
*/
inline float random() { return rand() / (float)RAND_MAX; }

/*
	randomRange - Returns a random value between the minimum (inclusive)
				  and the maximum (Exclusive)
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016

	param[in] pMin - The first value of type T used as the minimum 
	param[in] pMax - The second value of type T used as the maximum

	return T - Returns a random value between pMin and pMax
*/
template<typename T>
inline T randomRange(T pMin, T pMax) { return (T)(random() * (float)(pMax - pMin) + (float)pMin); }