#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <time.h>

#include "matrix4.h"

#ifndef DEGREE_TO_RADIANS
  #define DEGREE_TO_RADIANS(s) (s * (glm::pi<double>() / 180.0))
#endif

void Print (glm::mat4 mat);

class Trianglei {
  int vertex[3];
};

class Triangleui {
  unsigned int vertex[3];
};

class Trianglef {
  float vertex[3];
};

int Clamp (int value, int lower_bound, int higher_bound);
float Clamp (float value, float lower_bound, float higher_bound);

bool IsNaN (double x);

double Cotangent (double angle);

double DegreesToRadians(double degrees);
double RadiansToDegrees(double radians);

/*
xScale, 0,      0,                        0,
0,      yScale, 0,                        0,
0,      0,      (far + near) / nearmfar, -1,
0,      0,      2*far*near / nearmfar,    0
*/
Matrix4f CreateProjectionMatrix(float fovy, float aspect_ratio, float near_plane, float far_plane);

/**
* Returns:
* x: if x is between a and b
* a: if x is less than a
* b: if x is greater than b
**/
float Constrain(float x, float a, float b);

//http://en.wikipedia.org/wiki/Fast_inverse_square_root
float Q_rsqrt(float number);

//example: Average ( 2, 10.0, 4.0 ) 
double Average(int num, ...);

double DistanceManhattan(glm::dvec3 a, glm::dvec3 b);
double Distance(glm::dvec3 a, glm::dvec3 b);

double DistanceManhattan(glm::vec3 a, glm::vec3 b);
double Distance(glm::vec3 a, glm::vec3 b);

void TranslateMatrix(Matrix4f matrix, float wx, float hy, float dz);

glm::vec3 RodriguesRotation(glm::vec3 v, float teta, glm::vec3 k);
glm::dvec3 RodriguesRotation(glm::dvec3 v, double teta, glm::dvec3 k);

#endif