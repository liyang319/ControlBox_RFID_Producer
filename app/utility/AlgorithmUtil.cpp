#include "AlgorithmUtil.h"
#include "stdio.h"
#include "stdlib.h"
#include "Base.h"
#include <sstream>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <string>

using namespace std;

AlgorithmUtil::AlgorithmUtil(/* args */)
{
}

AlgorithmUtil::~AlgorithmUtil()
{
}

double AlgorithmUtil::nol(double x)
{
	if (32768 <= x)
	{
		x = x - 65536;
	}
	else
	{
		// do nothing
	}
	return x;
}

double AlgorithmUtil::Emeant(int l, double gam, double *qA)
{
	int k;
	double sum1;
	double sum2;

	sum1 = 0.0;
	sum2 = 0.0;

	for (k = 0; k < l; k++)
	{
		sum1 += pow(2, (-k * gam));
		sum2 += pow(2, (-k * gam)) * qA[k];
	}

	if (0 == sum1)
	{
		return 0;
	}
	else
	{
		return sum2 / sum1;
	}
}

double AlgorithmUtil::Dvart(int l, double gam, double *qA, double qE)
{
	int k;
	double sum1;
	double sum2;

	sum1 = 0.0;
	sum2 = 0.0;

	for (k = 0; k < l; k++)
	{
		sum1 += pow(2, (-k * gam));
		sum2 += pow(2, (-k * gam)) * pow((qA[k] - qE), 2);
	}

	if (0 == sum1)
	{
		return 0;
	}
	else
	{
		return sum2 / sum1;
	}
}

double AlgorithmUtil::maxDouble(double *pdouble, int len)
{
	int i;
	double tempd;

	tempd = 0;
	for (i = 0; i < len; i++)
	{
		if (tempd < *pdouble)
			tempd = *pdouble;
		pdouble++;
	}

	return tempd;
}

// double AlgorithmUtil::GetNewGamma2Ex(int gpssvnumsum, double gpsvavg, double gpsth, double azt, double azth, double gamma2)
// {
// 	double newGamma2 = 1.0;

// 	// 检查gamma2合法范围，确保合法
// 	if (gamma2 > 1)
//     {
// 		gamma2 = 1;
// 	}
//     else if (gamma2 < 0)
//     {
// 		gamma2 = 0;
// 	}

// 	if (gpssvnumsum < 0)
// 		gpssvnumsum = 0;

// 	if (gpssvnumsum > 0)
//     {
// 	    // 如果gps正常可用（有卫星数量）
// 	    if ((azt <= azth) && (gpsvavg <= gpsth))
//         {
// 	        if (1 == gamma2)
//             {
// 	            newGamma2 = 1;
// 	        }
//             else
//             {
// 	            newGamma2 = gamma2;
// 	        }
// 	    }
//         else
//         {
// 	        if (1 == gamma2)
//             {
// 	            newGamma2 = 0;
// 	        }
//             else
//             {
// 	            newGamma2 = gamma2 / 200;
// 	        }
// 	    }
// 	}
//     else
//     {
// 	    // 如果gps不可用（没有卫星）
// 	    if (azt <= azth)
//         {
// 	        newGamma2 = 1;
// 	    }
//         else
//         {
// 	        newGamma2 = 0;
// 	    }
// 	}

// 	return newGamma2;
// }

double AlgorithmUtil::getNewGamma2Ex(int gpssvnumsum, double gpsvavg, double gpsvcache, double GPSTH, double wcal, double WSTAND, double WTH, double WAC, double gamma2)
{
	double newGamma2 = 1.0;

	// ¼ì²égamma2ºÏ·¨·¶Î§£¬È·±£ºÏ·¨
	if (gamma2 > 1)
	{
		gamma2 = 1;
	}
	else if (gamma2 < 0)
	{
		gamma2 = 0;
	}

	if (gpssvnumsum > 0)
	{
		if ((gpsvavg <= GPSTH) && (gpsvcache <= GPSTH))
		{
			if (1 == gamma2)
			{
				newGamma2 = 1;
			}
			else if ((wcal >= WSTAND - WTH) && (wcal <= WSTAND + WTH))
			{
				newGamma2 = (gamma2 / WAC);
			}
			else
			{
				newGamma2 = gamma2;
			}
		}
		else
		{
			if (1 == gamma2)
			{
				newGamma2 = 0;
			}
			else
			{
				newGamma2 = (1 - gamma2) / 1000;
			}
		}
	}
	else
	{
		newGamma2 = gamma2;
	}

	return newGamma2;
}
