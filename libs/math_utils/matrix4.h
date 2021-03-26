#ifndef MATH_UTILS_MATRIX4_H
#define MATH_UTILS_MATRIX4_H

#include <cstring>

#include <cstdio>
#include <cstdlib>
#include "matrix.h"

#include <glm/glm.hpp>

#ifndef DEGTORAD
#define	DEGTORAD(x)	( ((x) * PI) / 180.0 )
#endif 

#ifndef PI
#define PI 3.14159265358979323846
#endif 

typedef struct Matrix4f
{
  static Matrix4f ConvertToMatrix4 (glm::mat4 mat)
  {
    Matrix4f m = { {
        mat[0][0], mat[0][1], mat[0][2], mat[0][3],
        mat[1][0], mat[1][1], mat[1][2], mat[1][3],
        mat[2][0], mat[2][1], mat[2][2], mat[2][3],
        mat[3][0], mat[3][1], mat[3][2], mat[3][3],
      } };
   //      Matrix4f m = { {
   //     mat[0][0], mat[1][0], mat[2][0], mat[3][0],
   //     mat[0][1], mat[1][1], mat[2][1], mat[3][1],
   //     mat[0][2], mat[1][2], mat[2][2], mat[3][2],
   //     mat[0][3], mat[1][3], mat[2][3], mat[3][3],
   //   } };
    return m;
  }

  void Identity ()
  {
    memset (m, 0, 16 * sizeof(float));		// set all to 0
    m[0] = 1;		// set diagonal to 1
    m[5] = 1;
    m[10] = 1;
    m[15] = 1;
  }

  void glm_mat4_copy (glm::mat4 mat)
  {

  }

  void rotacionaEixo (glm::vec3 axis, float angle)
  {
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float c = cosf (DEGTORAD (angle));
    float s = sinf (DEGTORAD (angle));

    //axis.print();

    m[0] = (x * x) * (1.0f - c) + c;
    m[1] = (x * y) * (1.0f - c) + (z * s);
    m[2] = (x * z) * (1.0f - c) - (y * s);
    m[3] = 0.0f;

    m[4] = (y * x) * (1.0f - c) - (z * s);
    m[5] = (y * y) * (1.0f - c) + c;
    m[6] = (y * z) * (1.0f - c) + (x * s);
    m[7] = 0.0f;

    m[8] = (z * x) * (1.0f - c) + (y * s);
    m[9] = (z * y) * (1.0f - c) - (x * s);
    m[10] = (z * z) * (1.0f - c) + c;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
  }

  float *getMatrix ()
  {
    return m;
  }

  Matrix4f operator*(const Matrix4f &m) const
  {
    Matrix4f result;		// temp vars
    double sum;
    int    index, alpha, beta;		// loop vars

    for (index = 0; index < 4; index++)			// perform multiplcation the slow and safe way
    {
      for (alpha = 0; alpha < 4; alpha++)
      {
        sum = 0.0f;
        for (beta = 0; beta < 4; beta++)
          sum += this->m[index + beta * 4] * m.m[alpha * 4 + beta];
        result.m[index + alpha * 4] = (float)sum;
      }
    }
    return(result);
  }

  //http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche23.html
  bool Invert3 ()
  {

    float a11 = m[0];
    float a12 = m[1];
    float a13 = m[2];
    float a14 = m[3];

    float a21 = m[4];
    float a22 = m[5];
    float a23 = m[6];
    float a24 = m[7];

    float a31 = m[8];
    float a32 = m[9];
    float a33 = m[10];
    float a34 = m[11];

    float a41 = m[12];
    float a42 = m[13];
    float a43 = m[14];
    float a44 = m[15];

    float detA = a11*a22*a33*a44 + a11*a23*a34*a42 + a11*a24*a32*a43
      + a12*a21*a34*a43 + a12*a23*a31*a44 + a12*a24*a33*a41
      + a13*a21*a32*a44 + a13*a22*a34*a41 + a13*a24*a31*a42
      + a14*a21*a33*a42 + a14*a22*a31*a43 + a14*a23*a32*a41
      - a11*a22*a34*a43 - a11*a23*a32*a44 - a11*a24*a33*a42
      - a12*a21*a33*a44 - a12*a23*a34*a41 - a12*a24*a31*a43
      - a13*a21*a34*a42 - a13*a22*a31*a44 - a13*a24*a32*a41
      - a14*a21*a32*a43 - a14*a22*a33*a41 - a14*a23*a31*a42;

    if (detA == 0.0f)
      return false;

    float b11 = a22*a33*a44 + a23*a34*a42 + a24*a32*a43 - a22*a34*a43 - a23*a32*a44 - a24*a33*a42;
    b11 = b11 / detA;

    float b12 = a12*a34*a43 + a13*a32*a44 + a14*a33*a42 - a12*a33*a44 - a13*a34*a42 - a14*a32*a43;
    b12 = b12 / detA;

    float b13 = a12*a23*a44 + a13*a24*a42 + a14*a22*a43 - a12*a24*a43 - a13*a22*a44 - a14*a23*a42;
    b13 = b13 / detA;

    float b14 = a12*a24*a33 + a13*a22*a34 + a14*a23*a32 - a12*a23*a34 - a13*a24*a32 - a14*a22*a33;
    b14 = b14 / detA;

    float b21 = a21*a34*a43 + a23*a31*a44 + a24*a33*a41 - a21*a33*a44 - a23*a34*a41 - a24*a31*a43;
    b21 = b21 / detA;

    float b22 = a11*a33*a44 + a13*a34*a41 + a14*a31*a43 - a11*a34*a43 - a13*a31*a44 - a14*a33*a41;
    b22 = b22 / detA;

    float b23 = a11*a24*a43 + a13*a21*a44 + a14*a23*a41 - a11*a23*a44 - a13*a24*a41 - a14*a21*a43;
    b23 = b23 / detA;

    float b24 = a11*a23*a34 + a13*a24*a31 + a14*a21*a33 - a11*a24*a33 - a13*a21*a34 - a14*a23*a31;
    b24 = b24 / detA;

    float b31 = a21*a32*a44 + a22*a34*a41 + a24*a31*a42 - a21*a34*a42 - a22*a31*a44 - a24*a32*a41;
    b31 = b31 / detA;

    float b32 = a11*a34*a42 + a12*a31*a44 + a14*a32*a41 - a11*a32*a44 - a12*a34*a41 - a14*a31*a42;
    b32 = b32 / detA;

    float b33 = a11*a22*a44 + a12*a24*a41 + a14*a21*a42 - a11*a24*a42 - a12*a21*a44 - a14*a22*a41;
    b33 = b33 / detA;

    float b34 = a11*a24*a32 + a12*a21*a34 + a14*a22*a31 - a11*a22*a34 - a12*a24*a31 - a14*a21*a32;
    b34 = b34 / detA;

    float b41 = a21*a33*a42 + a22*a31*a43 + a23*a32*a41 - a21*a32*a43 - a22*a33*a41 - a23*a31*a42;
    b41 = b41 / detA;

    float b42 = a11*a32*a43 + a12*a33*a41 + a13*a31*a42 - a11*a33*a42 - a12*a31*a43 - a13*a32*a41;
    b42 = b42 / detA;

    float b43 = a11*a23*a42 + a12*a21*a43 + a13*a22*a41 - a11*a22*a43 - a12*a23*a41 - a13*a21*a42;
    b43 = b43 / detA;

    float b44 = a11*a22*a33 + a12*a23*a31 + a13*a21*a32 - a11*a23*a32 - a12*a21*a33 - a13*a22*a31;
    b44 = b44 / detA;

    m[0] = b11; m[1] = b12; m[2] = b13; m[3] = b14;
    m[4] = b21; m[5] = b22; m[6] = b23; m[7] = b24;
    m[8] = b31; m[9] = b32; m[10] = b33; m[11] = b34;
    m[12] = b41; m[13] = b42; m[14] = b43; m[15] = b44;

    return true;
  }

  void Invert2 ()
  {
    float s0 = m[0] * m[5] - m[4] * m[1];
    float s1 = m[0] * m[6] - m[4] * m[2];
    float s2 = m[0] * m[7] - m[4] * m[3];
    float s3 = m[1] * m[6] - m[5] * m[2];
    float s4 = m[1] * m[7] - m[5] * m[3];
    float s5 = m[2] * m[7] - m[6] * m[3];

    float c5 = m[10] * m[15] - m[14] * m[11];
    float c4 = m[9] * m[15] - m[13] * m[11];
    float c3 = m[9] * m[14] - m[13] * m[10];
    float c2 = m[8] * m[15] - m[12] * m[11];
    float c1 = m[8] * m[14] - m[12] * m[10];
    float c0 = m[8] * m[13] - m[12] * m[9];

    // Should check for 0 determinant
    float invdet = 1.0f / (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);

    float b[16];

    b[0] = (m[5] * c5 - m[6] * c4 + m[7] * c3) * invdet;
    b[1] = (-m[1] * c5 + m[2] * c4 - m[3] * c3) * invdet;
    b[2] = (m[13] * s5 - m[14] * s4 + m[15] * s3) * invdet;
    b[3] = (-m[9] * s5 + m[10] * s4 - m[11] * s3) * invdet;

    b[4] = (-m[4] * c5 + m[6] * c2 - m[7] * c1) * invdet;
    b[5] = (m[0] * c5 - m[2] * c2 + m[3] * c1) * invdet;
    b[6] = (-m[12] * s5 + m[14] * s2 - m[15] * s1) * invdet;
    b[7] = (m[8] * s5 - m[10] * s2 + m[11] * s1) * invdet;

    b[8] = (m[4] * c4 - m[5] * c2 + m[7] * c0) * invdet;
    b[9] = (-m[0] * c4 + m[1] * c2 - m[3] * c0) * invdet;
    b[10] = (m[12] * s4 - m[13] * s2 + m[15] * s0) * invdet;
    b[11] = (-m[8] * s4 + m[9] * s2 - m[11] * s0) * invdet;

    b[12] = (-m[4] * c3 + m[5] * c1 - m[6] * c0) * invdet;
    b[13] = (m[0] * c3 - m[1] * c1 + m[2] * c0) * invdet;
    b[14] = (-m[12] * s3 + m[13] * s1 - m[14] * s0) * invdet;
    b[15] = (m[8] * s3 - m[9] * s1 + m[10] * s0) * invdet;

    for (int i = 0; i < 16; i++)
      m[i] = b[i];
  }

  bool Invert ()
  {
    float inv[16], det;
    int i;

    inv[0] = m[5] * m[10] * m[15] -
      m[5] * m[11] * m[14] -
      m[9] * m[6] * m[15] +
      m[9] * m[7] * m[14] +
      m[13] * m[6] * m[11] -
      m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] +
      m[4] * m[11] * m[14] +
      m[8] * m[6] * m[15] -
      m[8] * m[7] * m[14] -
      m[12] * m[6] * m[11] +
      m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] -
      m[4] * m[11] * m[13] -
      m[8] * m[5] * m[15] +
      m[8] * m[7] * m[13] +
      m[12] * m[5] * m[11] -
      m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] +
      m[4] * m[10] * m[13] +
      m[8] * m[5] * m[14] -
      m[8] * m[6] * m[13] -
      m[12] * m[5] * m[10] +
      m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] +
      m[1] * m[11] * m[14] +
      m[9] * m[2] * m[15] -
      m[9] * m[3] * m[14] -
      m[13] * m[2] * m[11] +
      m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] -
      m[0] * m[11] * m[14] -
      m[8] * m[2] * m[15] +
      m[8] * m[3] * m[14] +
      m[12] * m[2] * m[11] -
      m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] +
      m[0] * m[11] * m[13] +
      m[8] * m[1] * m[15] -
      m[8] * m[3] * m[13] -
      m[12] * m[1] * m[11] +
      m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] -
      m[0] * m[10] * m[13] -
      m[8] * m[1] * m[14] +
      m[8] * m[2] * m[13] +
      m[12] * m[1] * m[10] -
      m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] -
      m[1] * m[7] * m[14] -
      m[5] * m[2] * m[15] +
      m[5] * m[3] * m[14] +
      m[13] * m[2] * m[7] -
      m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] +
      m[0] * m[7] * m[14] +
      m[4] * m[2] * m[15] -
      m[4] * m[3] * m[14] -
      m[12] * m[2] * m[7] +
      m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] -
      m[0] * m[7] * m[13] -
      m[4] * m[1] * m[15] +
      m[4] * m[3] * m[13] +
      m[12] * m[1] * m[7] -
      m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] +
      m[0] * m[6] * m[13] +
      m[4] * m[1] * m[14] -
      m[4] * m[2] * m[13] -
      m[12] * m[1] * m[6] +
      m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
      m[1] * m[7] * m[10] +
      m[5] * m[2] * m[11] -
      m[5] * m[3] * m[10] -
      m[9] * m[2] * m[7] +
      m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
      m[0] * m[7] * m[10] -
      m[4] * m[2] * m[11] +
      m[4] * m[3] * m[10] +
      m[8] * m[2] * m[7] -
      m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
      m[0] * m[7] * m[9] +
      m[4] * m[1] * m[11] -
      m[4] * m[3] * m[9] -
      m[8] * m[1] * m[7] +
      m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
      m[0] * m[6] * m[9] -
      m[4] * m[1] * m[10] +
      m[4] * m[2] * m[9] +
      m[8] * m[1] * m[6] -
      m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0.0f)
      return false;

    det = 1.0f / det;

    for (i = 0; i < 16; i++)
      m[i] = inv[i] * det;

    return true;

  }

  void Invert (int mode)
  {
    if (mode == 0)
    {
      float m11 = m[0];
      float m21 = m[1];
      float m31 = m[2];
      float m41 = m[3];
      float m12 = m[4];
      float m22 = m[5];
      float m32 = m[6];
      float m42 = m[7];
      float m13 = m[8];
      float m23 = m[9];
      float m33 = m[10];
      float m43 = m[11];
      float m14 = m[12];
      float m24 = m[13];
      float m34 = m[14];
      float m44 = m[15];

      m[0] = (-m22*m33*m44 + m22*m34*m43 + m32*m23*m44 - m32*m24*m43 - m42*m23*m34 + m42*m24*m33) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44 +
        m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23*
        m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[4] = (m12*m33*m44 - m12*m34*m43 - m32*m13*m44 + m32*m14*m43 + m42*m13*m34 - m42*m14*m33) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44 +
        m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23*
        m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[8] = (-m12*m23*m44 + m12*m24*m43 + m22*m13*m44 - m22*m14*m43 - m42*m13*m24 + m42*m14*m23) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44 +
        m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23*
        m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[12] = -(-m12*m23*m34 + m12*m24*m33 + m22*m13*m34 - m22*m14*m33 - m32*m13*m24 + m32*m14*m23) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[1] = -(-m21*m33*m44 + m21*m34*m43 + m31*m23*m44 - m31*m24*m43 - m41*m23*m34 + m41*m24*m33) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[5] = -(m11*m33*m44 - m11*m34*m43 - m31*m13*m44 + m31*m14*m43 + m41*m13*m34 - m41*m14*m33) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[9] = -(-m11*m23*m44 + m11*m24*m43 + m21*m13*m44 - m21*m14*m43 - m41*m13*m24 + m41*m14*m23) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[13] = (-m11*m23*m34 + m11*m24*m33 + m21*m13*m34 - m21*m14*m33 - m31*m13*m24 + m31*m14*m23) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[2] = -(m21*m32*m44 - m21*m34*m42 - m31*m22*m44 + m31*m24*m42 + m41*m22*m34 - m41*m24*m32) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[6] = (m11*m32*m44 - m11*m34*m42 - m31*m12*m44 + m31*m14*m42 + m41*m12*m34 - m41*m14*m32) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[10] = -(m11*m22*m44 - m11*m24*m42 - m21*m12*m44 + m21*m14*m42 + m41*m12*m24 - m41*m14*m22) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[14] = (m11*m22*m34 - m11*m24*m32 - m21*m12*m34 + m21*m14*m32 + m31*m12*m24 - m31*m14*m22) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[3] = (m21*m32*m43 - m21*m33*m42 - m31*m22*m43 + m31*m23*m42 + m41*m22*m33 - m41*m23*m32) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[7] = -(m11*m32*m43 - m11*m33*m42 - m31*m12*m43 + m31*m13*m42 + m41*m12*m33 - m41*m13*m32) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[11] = (m11*m22*m43 - m11*m23*m42 - m21*m12*m43 + m21*m13*m42 + m41*m12*m23 - m41*m13*m22) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);

      m[15] = -(m11*m22*m33 - m11*m23*m32 - m21*m12*m33 + m21*m13*m32 + m31*m12*m23 - m31*m13*m22) /
        (-m11*m22*m33*m44 + m11*m22*m34*m43 + m11*m32*m23*m44 - m11*m32*m24*m43 -
        m11*m42*m23*m34 + m11*m42*m24*m33 + m21*m12*m33*m44 - m21*m12*m34*m43 - m21*m32*m13*m44
        + m21*m32*m14*m43 + m21*m42*m13*m34 - m21*m42*m14*m33 - m31*m12*m23*m44 + m31*m12*m24*
        m43 + m31*m22*m13*m44 - m31*m22*m14*m43 - m31*m42*m13*m24 + m31*m42*m14*m23 + m41*m12*m23
        *m34 - m41*m12*m24*m33 - m41*m22*m13*m34 + m41*m22*m14*m33 + m41*m32*m13*m24 - m41*m32*m14*m23);
    }
    else if (mode == 1)
    {
      float Adj[16];    // Cofactors and adjoint matrix
      float Det2x2[12]; // 2x2 sub matrix determinants

      float m0 = m[0];
      float m1 = m[1];
      float m2 = m[2];
      float m3 = m[3];
      float m4 = m[4];
      float m5 = m[5];
      float m6 = m[6];
      float m7 = m[7];
      float m8 = m[8];
      float m9 = m[9];
      float m10 = m[10];
      float m11 = m[11];
      float m12 = m[12];
      float m13 = m[13];
      float m14 = m[14];
      float m15 = m[15];

      // Auxiliary determinants of 2x2 sub-matrices

      Det2x2[0] = m10 * m15 - m11 * m14;
      Det2x2[1] = m9  * m15 - m11 * m13;
      Det2x2[2] = m9  * m14 - m10 * m13;
      Det2x2[3] = m8  * m15 - m11 * m12;
      Det2x2[4] = m8  * m14 - m10 * m12;
      Det2x2[5] = m8  * m13 - m9  * m12;

      // First 4 cofactors

      Adj[0] = m5 * Det2x2[0] - m6 * Det2x2[1] + m7 * Det2x2[2];
      Adj[1] = -(m4 * Det2x2[0] - m6 * Det2x2[3] + m7 * Det2x2[4]);
      Adj[2] = m4 * Det2x2[1] - m5 * Det2x2[3] + m7 * Det2x2[5];
      Adj[3] = -(m4 * Det2x2[2] - m5 * Det2x2[4] + m6 * Det2x2[5]);

      // Calculation and verification of the 4x4 matrix determinant

      float Det4x4 = m0 * Adj[0]
        + m1 * Adj[1]
        + m2 * Adj[2]
        + m3 * Adj[3];

      if (Det4x4 == 0.0f)     // The matrix is not invertible! Return a matrix filled with zeros
      {
        m[0] = 0.0f; m[4] = 0.0f; m[8] = 0.0f; m[12] = 0.0f;
        m[1] = 0.0f; m[5] = 0.0f; m[9] = 0.0f; m[13] = 0.0f;
        m[2] = 0.0f; m[6] = 0.0f; m[10] = 0.0f; m[14] = 0.0f;
        m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 0.0f;
        return;
      }

      // Other auxiliary determinants of 2x2 sub-matrices

      Det2x2[6] = m2  * m7 - m3  * m6;
      Det2x2[7] = m1  * m7 - m3  * m5;
      Det2x2[8] = m1  * m6 - m2  * m5;
      Det2x2[9] = m0  * m7 - m3  * m4;
      Det2x2[10] = m0  * m6 - m2  * m4;
      Det2x2[11] = m0  * m5 - m1  * m4;

      // Other 12 cofactors

      Adj[4] = -(m1 * Det2x2[0] - m2 * Det2x2[1] + m3 * Det2x2[2]);
      Adj[5] = m0 * Det2x2[0] - m2 * Det2x2[3] + m3 * Det2x2[4];
      Adj[6] = -(m0 * Det2x2[1] - m1 * Det2x2[3] + m3 * Det2x2[5]);
      Adj[7] = m0 * Det2x2[2] - m1 * Det2x2[4] + m2 * Det2x2[5];

      Adj[8] = m13* Det2x2[6] - m14* Det2x2[7] + m15* Det2x2[8];
      Adj[9] = -(m12* Det2x2[6] - m14* Det2x2[9] + m15* Det2x2[10]);
      Adj[10] = m12* Det2x2[7] - m13* Det2x2[9] + m15* Det2x2[11];
      Adj[11] = -(m12* Det2x2[8] - m13* Det2x2[10] + m14* Det2x2[11]);
      Adj[12] = -(m9 * Det2x2[6] - m10* Det2x2[7] + m11* Det2x2[8]);
      Adj[13] = m8 * Det2x2[6] - m10* Det2x2[9] + m11* Det2x2[10];
      Adj[14] = -(m8 * Det2x2[7] - m9 * Det2x2[9] + m11* Det2x2[11]);
      Adj[15] = m8 * Det2x2[8] - m9 * Det2x2[10] + m10* Det2x2[11];

      // Finding the transposed adjoint matrix, which is the inverse

      float IDet = (1.0f / Det4x4);

      m[0] = IDet * Adj[0];  m[4] = IDet * Adj[1];  m[8] = IDet * Adj[2];  m[12] = IDet * Adj[3];
      m[1] = IDet * Adj[4];  m[5] = IDet * Adj[5];  m[9] = IDet * Adj[6];  m[13] = IDet * Adj[7];
      m[2] = IDet * Adj[8];  m[6] = IDet * Adj[9];  m[10] = IDet * Adj[10]; m[14] = IDet * Adj[11];
      m[3] = IDet * Adj[12]; m[7] = IDet * Adj[13]; m[11] = IDet * Adj[14]; m[15] = IDet * Adj[15];
    }

  }

  Matrix4f Inverse (int mode = 1)
  {
    Matrix4f inverse = *this;
    inverse.Invert (mode);
    return inverse;
  }

  Matrix4f& operator=(const Matrix4f& m)
  {
    for (int i = 0; i < 16; i++)
      this->m[i] = m.m[i];
    return *this;
  }

  float m[16];
} Matrix4f;

static const Matrix4f IDENTITY_MATRIX = {
  {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  }
};

/*Mat<float, 4, 4> rotation3Dx (float graus)
{
  Mat<float, 4, 4> ret;
  ret.Zero ();
  ret (0, 0) = 1;
  ret (1, 1) = cos (graus);
  ret (1, 2) = -sin (graus);
  ret (2, 1) = sin (graus);
  ret (2, 2) = cos (graus);
  ret (3, 3) = 1;
  return ret;
}

Matriz<4, 4> rotation3Dy (float graus)
{
  Matriz<4, 4> ret;
  ret.setZero ();
  ret (0, 0) = cos (graus);
  ret (0, 2) = sin (graus);
  ret (1, 1) = 1;
  ret (2, 0) = -sin (graus);
  ret (2, 2) = cos (graus);
  ret (3, 3) = 1;
  return ret;
}

Matriz<4, 4> rotation3Dz (float graus)
{
  Matriz<4, 4> ret;
  ret.setZero ();
  ret (0, 0) = cos (graus);
  ret (0, 1) = -sin (graus);
  ret (1, 0) = sin (graus);
  ret (1, 1) = cos (graus);
  ret (2, 2) = 1;
  ret (3, 3) = 1;
  return ret;
}*/

#endif