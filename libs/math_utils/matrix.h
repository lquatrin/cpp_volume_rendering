#ifndef MATH_UTILS_T_MATRIX_H
#define MATH_UTILS_T_MATRIX_H

#include <cstdlib>
#include <cassert>

template<typename T>
class Matrix
{
public:
  Matrix ()
    : m_rows (0), m_cols (0)
  {
    m = NULL;
  }

  Matrix (int rows, int cols)
    : m_rows (rows), m_cols (cols)
  {
    m = new T[rows*cols] ();
  }

  ~Matrix ()
  {
    delete[] m;
  }

  int GetRows ()
  {
    return m_rows;
  }

  int GetCols ()
  {
    return m_cols;
  }

  int GetColumns ()
  {
    return GetCols ();
  }

  bool Set(T value)
  {
    for (int r = 0; r < GetRows(); r++)
      for (int c = 0; c < GetColumns(); c++)
        Set(value, r, c);

    return true;
  }

  bool Set (T value, int row, int col)
  {
    if (IsOutOfBounds (row, col))
      return false;
    m[(row*m_cols) + col] = value;
  }

  T Get (int row, int col)
  {
    if (IsOutOfBounds (row, col))
      return -1.f;
    return m[(row*m_cols) + col];
  }

  bool IsOutOfBounds (int row, int col)
  {
    return (row < 0 || col < 0 || row >= m_rows || col >= m_cols);
  }

  T& at (int r, int c)
  {
    assert (!IsOutOfBounds (r, c));
    return m[(r*m_cols) + c];
  }

  const T& at (int r, int c) const
  {
    assert (!IsOutOfBounds (r, c));
    return m[(r*m_cols) + c];
  }

  T& operator() (int r, int c)
  {
    assert (!IsOutOfBounds (r, c));
    return m[(r*m_cols) + c];
  }

  const T& operator() (int r, int c) const
  {
    assert (!IsOutOfBounds (r, c));
    return m[(r*m_cols) + c];
  }

private:
  template<typename T>
  class ProxyOpMatrix
  {
  public:
    ProxyOpMatrix (Matrix<T> *ptr, int row)
    {
      m_ptr = ptr;
      m_row = row;
    }

    T& operator[] (int c)
    {
      return m_ptr->at (m_row, c);
    }

  private:
    Matrix<T> *m_ptr;
    int m_row;
  };

  template<typename T>
  class ConstProxyOpMatrix
  {
  public:
    ConstProxyOpMatrix (const Matrix<T> *ptr, int row)
    {
      m_ptr = ptr;
      m_row = row;
    }

    T operator[] (int c)
    {
      return m_ptr->at (m_row, c);
    }

  private:
    const Matrix<T> *m_ptr;
    int m_row;
  };

  int m_rows, m_cols;
  T* m;

public:
  ProxyOpMatrix<T> operator[] (int r)
  {
    return ProxyOpMatrix<T> (this, r);
  }

  ConstProxyOpMatrix<T> operator[] (int r) const
  {
    return ConstProxyOpMatrix<T> (this, r);
  }
};

template<typename T, unsigned int R, unsigned int C = R>
class Mat
{
public:
  Mat ()
  {}

  ~Mat ()
  {}

  void Zero ()
  {
    for (int i = 0; i < R*C; i++)
    {
      m[i] = T (0);
    }
  }

  int GetRows ()
  {
    return R;
  }

  int GetCols ()
  {
    return C;
  }

  int GetColumns ()
  {
    return GetCols ();
  }

  bool Set (T value, int row, int col)
  {
    if (IsOutOfBounds (row, col))
      return false;
    m[(row*C) + col] = value;
  }

  T Get (int row, int col)
  {
    if (IsOutOfBounds (row, col))
      return -1.f;
    return m[(row*C) + col];
  }

  bool IsOutOfBounds (int row, int col)
  {
    return (row < 0 || col < 0 || row >= R || col >= C);
  }

  T& at (int r, int c)
  {
    assert (!IsOutOfBounds (r, c));
    return m[(r*C) + c];
  }

  const T& at (int r, int c) const
  {
    assert (!IsOutOfBounds (r, c));
    return m[(r*C) + c];
  }

  T& operator() (int r, int c)
  {
    assert (!IsOutOfBounds (r, c));
    return m[(r*C) + c];
  }

  const T& operator() (int r, int c) const
  {
    assert (!IsOutOfBounds (r, c));
    return m[(r*C) + c];
  }
  T m[R*C];
private:
  template<typename T, unsigned int R, unsigned int C = R>
  class ProxyOpMat
  {
  public:
    ProxyOpMat (Mat<T, R, C> *ptr, int row)
    {
      m_ptr = ptr;
      m_row = row;
    }

    T& operator[] (int c)
    {
      return m_ptr->at (m_row, c);
    }

  private:
    Mat<T, R, C> *m_ptr;
    int m_row;
  };

  template<typename T, unsigned int R, unsigned int C = R>
  class ConstProxyOpMat
  {
  public:
    ConstProxyOpMat (const Mat<T, R, C> *ptr, int row)
    {
      m_ptr = ptr;
      m_row = row;
    }

    T operator[] (int c)
    {
      return m_ptr->at (m_row, c);
    }

  private:
    const Mat<T, R, C> *m_ptr;
    int m_row;
  };

public:
  ProxyOpMat<T, R, C> operator[] (int r)
  {
    return ProxyOpMat<T, R, C> (this, r);
  }

  ConstProxyOpMat<T, R, C> operator[] (int r) const
  {
    return ConstProxyOpMat<T, R, C> (this, r);
  }
};

typedef Mat<int, 2> Matrix2i;
typedef Mat<float, 2> Matrix2f;
typedef Mat<double, 2> Matrix2d;

typedef Mat<int, 3> Matrix3i;
typedef Mat<float, 3> Matrix3f;
typedef Mat<double, 3> Matrix3d;

typedef Mat<int, 4> Matrix4i;
//typedef Mat<float, 4> Matrix4f;
typedef Mat<double, 4> Matrix4d;

#endif