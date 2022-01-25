#pragma once

#include <math.h>
#include <cassert>
#include <vector>
#include <string>

/** Describes a parameter range to be used as a dimension for a parameter space.
* 
*   A parameter range relates to a given parameter and is able to change it
*   within a given start and end value with a given step size.
* 
*   The details are implemented in derived classes.
*/
class ParameterRangeBase
{
//Construction / Deconstruction
public:
  ParameterRangeBase(const std::string& name)
    :m_name(name)
  {}

  virtual ~ParameterRangeBase()
  {}

//Functions
public:
  ///Reset the internal counter to point to the beginning of the parameter range.
  virtual void Start() = 0;

  ///Increase the internal counter by one step.
  virtual void Incr() = 0;

  ///Test whether the internal counter reached the end of the parameter range.
  virtual bool End() const = 0;

  ///Returns the number of steps in this range.
  virtual int NumSteps() const = 0;

  ///Store the current value to be restored later with RestoreCurrentValue()
  virtual void SaveCurrentValue() = 0;

  ///Restores the current value from the last call to SaveCurrentValue()
  virtual void RestoreCurrentValue() = 0;

  ///Returns the name of the parameter
  const std::string& GetName() const {return m_name;};

  ///Returns the current value as a string
  virtual std::string GetValueStr() const = 0;

//Attributes
protected:
  ///Name of this range
  std::string m_name;
};


/** Describes a numeric parameter range.
* 
*   @see ParameterRangeBase
*/
template<typename T>
class ParameterRangeNumeric : public ParameterRangeBase
{
//Construction / Deconstruction
public:
  /** Creates a numeric parameter range between [@c start, @c end]
  *   with a step size of @c incr.
  *   The variable pointed to by @c param will be affected by this range.
  *   Make sure the memory for @c param is allocated throughout the lifetime
  *   of this parameter range object.
  * 
  *   The number of steps in this range are: 1 + ceil((@c end - @c start) / @incr)
  */
  ParameterRangeNumeric(const std::string& name, T* param, const T start, const T end, const T incr)
    :ParameterRangeBase(name)
    ,m_start(0)
    ,m_end(2)
    ,m_incr(1)
    ,m_curr(NULL)
  {
    Set(start, end, incr, param);
  }

  virtual ~ParameterRangeNumeric()
  {}

//Functions
public:
  ///Set the values for the range. This also resets the counter to the beginning of the range.
  void Set(const T start, const T end, const T incr, T* param)
  {
    //We do not accept inverted ranges or values that lead to infinite ranges.
    if (start > end || incr <= 0 || !param)
    {
      throw;
    }

    //Set them.
    m_start = start;
    m_end = end;
    m_incr = incr;
    m_curr = param;
  }

  ///Reset the internal counter to point to the beginning of the parameter range.
  virtual void Start() override
  {
    assert(m_curr);
    *m_curr = m_start;
  }

  ///Test whether the internal counter reached the end of the parameter range.
  virtual bool End() const override
  {
    assert(m_curr);
    return (*m_curr > m_end);
  }

  ///Increase the internal counter by one step.
  virtual void Incr() override
  {
    assert(m_curr);
    *m_curr += m_incr;
  }

  ///Store the current value to be restored later with RestoreCurrentValue()
  virtual void SaveCurrentValue() override
  {
    m_previousvalue = *m_curr;
  }

  ///Restores the current value from the last call to SaveCurrentValue()
  virtual void RestoreCurrentValue() override
  {
    *m_curr = m_previousvalue;
  }

  ///Returns the number of steps in this range.
  virtual int NumSteps() const override
  {
    return 1 + (int)ceil((m_end - m_start) / m_incr);
  }

  ///Returns the current value as a string
  virtual std::string GetValueStr() const override
  {
    return std::to_string(*m_curr);
  }

//Attributes
protected:
  T m_start;
  T m_end;
  T m_incr;
  T m_previousvalue;
  T* m_curr;
};


using ParameterRangeFloat = ParameterRangeNumeric<float>;
using ParameterRangeDouble = ParameterRangeNumeric<double>;
using ParameterRangeInt = ParameterRangeNumeric<int>;


/** Defines a parameter space for evaluation of an algorithm.
* 
*/
class ParameterSpace
{
//Construction / Deconstruction
public:
  ParameterSpace();
  virtual ~ParameterSpace();

//Functions
public:
  ///Adds a parameter to this space.
  ///Ownership of @c param is assumed.
  void AddParameterDimension(ParameterRangeBase* param);

  ///Removes all dimensions.
  void ClearParameterDimensions();

  ///The number of dimensions.
  int GetNumDimensions() const;

  ///Returns the name of a dimension identified by index.
  /// This can be used as a header of a csv file.
  const std::string& GetDimensionName(const int idx) const;

  ///Returns the current value of a dimension identified by index.
  /// This can be used as the content of a csv file.
  const std::string GetDimensionValue(const int idx) const;

  ///The number of sample points of the parameter space.
  int GetNumSamplePoints() const {return m_numsamples_cached;};

  ///Starts an evaluation
  void StartEvaluation();

  ///Ends an evaluation and restores previous values
  void EndEvaluation();

  ///Increment during evaluation to the next sample point.
  bool IncrEvaluation();

protected:
  ///The number of sample points of the parameter space.
  int ComputeNumSamplePoints();

//Attributes
protected:
  ///The dimensions of the parameter space
  std::vector<ParameterRangeBase*> m_dimensions;

private:
  //Cached value of sample points.
  int m_numsamples_cached;
};


//ParameterSpaceEvaluation <== function maybe


bool ParameterSpaceTest();