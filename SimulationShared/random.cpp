#include "stdafx.h"

#include <stdlib.h>
#include "macros.h"
#include "random.h"
float getRangedRandom(float min, float max)
{
    int r = rand();

    float r1 = (float)r/(float)RAND_MAX;

    r1 *= (max-min);

    r1 += min;

    return r1;
}
int getRangedRandom(int min, int max)
{
    int r = rand();

    float r2 = (float)r/(float)RAND_MAX;

    r2 *= ((float)max-(float)min + 0.99f);
	int r1 = (int) r2;

    r1 += min;

    return r1;

}