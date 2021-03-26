#include "stackmatrix.h"

#include <glm/gtc/type_ptr.hpp>

namespace gl
{
  StackMatrix::StackMatrix ()
  {
    m_currentmode = MODELVIEW;
    m_stk_modelview.push(glm::mat4(1.0f));
    m_stk_projection.push(glm::mat4(1.0f));
    m_stk_texture.push(glm::mat4(1.0f));
  }

  StackMatrix::~StackMatrix ()
  {}

  void StackMatrix::MatrixMode (STACK mode)
  {
    m_currentmode = mode;
  }

  void StackMatrix::LoadIdentity ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack();
    if (!current_stack)
    {
      printf("lqc: current stack not found\n");
      return;
    }
    // Pop the last matrix and push a new identity
    current_stack->pop();
    current_stack->push(glm::mat4(1.0f));
  }

  void StackMatrix::MultiplyMatrix (glm::mat4 m, bool rightmultiplication)
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack();
    if (!current_stack)
    {
      printf("lqc: Current stack not found\n");
      return;
    }
    glm::mat4 stack_m = current_stack->top();
    current_stack->pop();
    //é na ordem contrária da aplicação a multiplicação das matrizes
    if (rightmultiplication)
      current_stack->push(stack_m * m);
    else
      current_stack->push(m * stack_m);
  }

  void StackMatrix::SetCurrentMatrix (glm::mat4 m)
  {
    GetCurrentStack()->pop();
    GetCurrentStack()->push(m);
  }

  void StackMatrix::PopMatrix ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack();
    if (!current_stack)
    {
      printf("lqc: Current stack not found\n");
      return;
    }
    current_stack->pop();
  }

  void StackMatrix::PushMatrix ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack();
    if (!current_stack)
    {
      printf("lqc: Current stack not found\n");
      return;
    }
    glm::mat4 top = current_stack->top();
    current_stack->push(top);
  }

  void StackMatrix::ResetStack ()
  {
    switch (m_currentmode)
    {
    case MODELVIEW:
      m_stk_modelview = std::stack<glm::mat4>();
      m_stk_modelview.push(glm::mat4(1.0f));
      break;
    case PROJECTION:
      m_stk_projection = std::stack<glm::mat4>();
      m_stk_projection.push(glm::mat4(1.0f));
      break;
    case TEXTURE:
      m_stk_texture = std::stack<glm::mat4>();
      m_stk_texture.push(glm::mat4(1.0f));
      break;
    default:
      printf("lqc: Matrix_Mode Unknown at GLStackMatrix.ResetMatrix()\n");
      break;
    }
  }

  void StackMatrix::PushIdentity ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack();
    if (!current_stack)
    {
      printf("lqc: Current stack not found\n");
      return;
    }
    current_stack->push(glm::mat4(1.0f));
  }

  glm::mat4 StackMatrix::GetMatrix ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack();
    if (!current_stack)
    {
      printf("lqc: Current stack not found, returning IDENTITY_MATRIX\n");
      return glm::mat4(1.0f);
    }
    return current_stack->top();
  }

  glm::mat4 StackMatrix::GetMatrix (StackMatrix::STACK matrix_mode)
  {
    switch (matrix_mode)
    {
    case MODELVIEW:
      return m_stk_modelview.top();
      break;
    case PROJECTION:
      return m_stk_projection.top();
      break;
    case TEXTURE:
      return m_stk_texture.top();
      break;
    }
    return glm::mat4(1.0f);
  }

  float* StackMatrix::GetMatrixPtr ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack();
    if (!current_stack)
    {
      printf("lqc: Current stack not found, returning IDENTITY_MATRIX\n");
      glm::mat4 m_identity = glm::mat4(1.0f);
      return glm::value_ptr(m_identity);
    }
    return glm::value_ptr(current_stack->top());
  }

  float* StackMatrix::GetMatrixPtr (StackMatrix::STACK matrix_mode)
  {
    switch (matrix_mode)
    {
    case MODELVIEW:
      return glm::value_ptr(m_stk_modelview.top());
      break;
    case PROJECTION:
      return glm::value_ptr(m_stk_projection.top());
      break;
    case TEXTURE:
      return glm::value_ptr(m_stk_texture.top());
      break;
    }
    glm::mat4 m_identity = glm::mat4(1.0f);
    return glm::value_ptr(m_identity);
  }

  glm::mat4 StackMatrix::PopAndGetMatrix ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack();
    if (!current_stack)
    {
      printf("lqc: Current stack not found, returning IDENTITY_MATRIX\n");
      return glm::mat4(1.0f);
    }
    glm::mat4 ret = current_stack->top();
    current_stack->pop();
    return ret;
  }

  std::stack<glm::mat4>* StackMatrix::GetCurrentStack ()
  {
    switch (m_currentmode)
    {
    case MODELVIEW:
      return &m_stk_modelview;
    case PROJECTION:
      return &m_stk_projection;
    case TEXTURE:
      return &m_stk_texture;
    default:
      printf("lqc: Current Matrix_Mode Unknown at GLStackMatrix.GetCurrentStack() function\n");
      return NULL;
    }
  }
}