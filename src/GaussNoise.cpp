//----------------------------------------------------------------------
// GaussNoise.cpp:
//   Implementation of functions to generate Gauss Noise
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <math.h>
#include <memory.h>
#include <stdlib.h>

#include "GaussNoise.h"
#include "ConstVal.h"

double CorValue(int diff, int Interval);

void CalculateCovar(int dim, int Interval, double CovarMatrix[])
{
	int i, j, k, end;
	double *p = CovarMatrix, *p1, *p2;
	double diag, value;

	memset(CovarMatrix, 0, sizeof(double) * SUM_N(dim));
	// first column
	for (i = 0; i < Interval; i ++)
	{
		*p++ = CorValue(i, Interval);
		p += i;
	}

	for (i = 1; i < dim; i ++)
	{
		// diagonal element
		p = CovarMatrix + DIAG_INDEX(i);
		p1 = CovarMatrix + SUM_N(i);
		diag = 1.0;
		for (k = 0; k < i; k ++)
		{
			diag -= (*p1) * (*p1);
			p1 ++;
		}
		diag = sqrt(diag);
		*p++ = diag;
		p += i;

		end = i + Interval;
		end = (end > dim) ? dim : end;
		for (j = i + 1; j < end; j ++)
		{
			p1 = CovarMatrix + SUM_N(i);
			p2 = CovarMatrix + SUM_N(j);
			value = CorValue(i - j, Interval);
			for (k = 0; k < i; k ++)
			{
				value -= (*p1) * (*p2);
				p1 ++; p2 ++;
			}
			*p++ = value / diag;
			p += j;
		}
	}
}

void GenerateNoise(int dim, int max_index, double CovarMatrix[], double Sigma, double NoiseI[], double NoiseQ[])
{
	int i, j, start;
	int value1, value2;
	double noise_i[MAX_DIM], noise_q[MAX_DIM];
	double *p;

	// first generate normalized complex Gauss noise
	for (i = 0; i < dim; i ++)
	{
		value1 = rand();
		value2 = rand();
		if (value1 == 0)	// prevent overflow
			value1 = 1;
		// scale noise power to be 1
		noise_i[i] = sqrt(-log((double)value1 / (RAND_MAX + 1)) * 2) * cos(PI2 * value2 / (RAND_MAX + 1));
		noise_q[i] = sqrt(-log((double)value1 / (RAND_MAX + 1)) * 2) * sin(PI2 * value2 / (RAND_MAX + 1));
	}

	// generate relative noise
	for (i = 0; i < dim; i ++)
	{
		start = i - max_index + 1;
		if (start < 0) start = 0;
		NoiseI[i] = NoiseQ[i] = 0.0;
		p = CovarMatrix + SUM_N(i) + start;
		for (j = start; j <= i; j ++, p ++)
		{
			NoiseI[i] += noise_i[j] * (*p);
			NoiseQ[i] += noise_q[j] * (*p);
		}
	}

	// scale
	for (i = 0; i < dim; i ++)
	{
		NoiseI[i] *= Sigma;
		NoiseQ[i] *= Sigma;
	}
}

double CorValue(int diff, int Interval)
{
	diff = (diff >= 0) ? diff : -diff;
	return (diff >= Interval) ? 0.0 : ((diff == 0) ? 1.0 : 1 - (double)diff / Interval);
}
