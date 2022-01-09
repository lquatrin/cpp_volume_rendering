#include "parameterspace.h"

ParameterSpace::ParameterSpace()
{
}

ParameterSpace::~ParameterSpace()
{
  //We assumed ownership of the dimensions in AddParameterDimension()
  ClearParameterDimensions();
}

void ParameterSpace::AddParameterDimension(ParameterRangeBase* param)
{
  m_dimensions.push_back(param);
  ComputeNumSamplePoints();
}

void ParameterSpace::ClearParameterDimensions()
{
  //We assumed ownership of the dimensions in AddParameterDimension()
  for(auto it=m_dimensions.begin();it!=m_dimensions.end();it++)
  {
    delete *it;
  }
  m_dimensions.clear();

  ComputeNumSamplePoints();
}

int ParameterSpace::GetNumDimensions() const
{
  return (int)m_dimensions.size();
}

const std::string& ParameterSpace::GetDimensionName(const int idx) const
{
  assert(idx >=0 && idx < m_dimensions.size());
  return m_dimensions[idx]->GetName();
}

const std::string ParameterSpace::GetDimensionValue(const int idx) const
{
  assert(idx >=0 && idx < m_dimensions.size());
  return m_dimensions[idx]->GetValueStr();
}

int ParameterSpace::ComputeNumSamplePoints()
{
  m_numsamples_cached = 0;
  if (m_dimensions.empty()) return m_numsamples_cached;

  m_numsamples_cached = 1;
  for(auto it=m_dimensions.begin();it!=m_dimensions.end();it++)
  {
    m_numsamples_cached *= (*it)->NumSteps();
  }

  return m_numsamples_cached;
}

void ParameterSpace::StartEvaluation()
{
  //Reset all dimensions to their starting point and save their current values.
  for(auto it=m_dimensions.begin();it!=m_dimensions.end();it++)
  {
    (*it)->SaveCurrentValue();
    (*it)->Start();
  }
}

void ParameterSpace::EndEvaluation()
{
  //Reset all dimensions to their previous values.
  for(auto it=m_dimensions.begin();it!=m_dimensions.end();it++)
  {
    (*it)->RestoreCurrentValue();
  }
}

bool ParameterSpace::IncrEvaluation()
{
  int dim = m_dimensions.size() - 1;

  if (dim < 0) return false;

  //Increase the last dimension first
  do
  {
    m_dimensions[dim]->Incr();
    if (m_dimensions[dim]->End())
    {
      m_dimensions[dim]->Start();
      dim--;
    }
    else
    {
      break;
    }
  }
  while (dim >= 0);

  //false, if we reached the end of the parameter space.
  return (dim >= 0);
}


bool ParameterSpaceTest()
{
  bool bResult(true);


  double Test;
  ParameterRangeDouble dParam("DTest", &Test, 0, 1, 0.1);
  int i(0);
  for(dParam.Start();!dParam.End();dParam.Incr())
  {
    i++;
  }
  bResult &= (i == 11 && dParam.NumSteps() == 11);

  int iTest;
  ParameterRangeInt iParam("IntTest", &iTest, 0, 10, 1);
  int j(0);
  for(iParam.Start();!iParam.End();iParam.Incr())
  {
    j++;
  }
  bResult &= (j == 11 && iParam.NumSteps() == 11);


  ParameterSpace PS;
  PS.AddParameterDimension(new ParameterRangeDouble(dParam));
  PS.AddParameterDimension(new ParameterRangeInt(iParam));

  bResult &= (PS.GetNumSamplePoints() == i * j);

  printf("%s and %s\n", PS.GetDimensionName(0).c_str(), PS.GetDimensionName(1).c_str());

  PS.StartEvaluation();
  do
  {
    printf("%s and %s\n", PS.GetDimensionValue(0).c_str(), PS.GetDimensionValue(1).c_str());
  }
  while (PS.IncrEvaluation());

  return bResult;
}

