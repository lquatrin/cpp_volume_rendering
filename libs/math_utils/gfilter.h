#ifndef MATH_UTILS_GAUSSIAN_FILTER_H
#define MATH_UTILS_GAUSSIAN_FILTER_H

double GaussianFilter (double sig, double x, bool normalize = true);
double GaussianFilter (double sig, double x, double y, bool normalize = true);
double GaussianFilter (double sig, double x, double y, double z, bool normalize = true);

#endif
