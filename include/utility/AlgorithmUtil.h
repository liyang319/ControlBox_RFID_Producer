#ifndef ALGORITHM_UTIL_H
#define ALGORITHM_UTIL_H

#include <cstddef>

class AlgorithmUtil
{
private:
    /* data */
public:
    AlgorithmUtil(/* args */);
    ~AlgorithmUtil();

    static double nol(double x);
    static double Emeant(int l, double gam, double *qA);
    static double Dvart(int l, double gam, double *qA, double qE);
    static double maxDouble(double *pdouble, int len);
    // static double GetNewGamma2Ex(int gpssvnumsum, double gpsvavg, double gpsth, double azt, double azth, double gamma2);
    static double getNewGamma2Ex(int gpssvnumsum, double gpsvavg, double gpsvcache, double GPSTH, double wcal, double WSTAND, double WTH, double WAC, double gamma2);
};

#endif