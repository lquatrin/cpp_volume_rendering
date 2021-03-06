#include "layeredframebufferobject.h"

#include <iostream>
#include <gl_utils/utils.h>

#include <GL/glew.h>

void LayeredFrameBufferObject::Unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LayeredFrameBufferObject::Bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

LayeredFrameBufferObject::LayeredFrameBufferObject (GLuint custom_number_of_attachments)
  : cur_number_of_attachments(custom_number_of_attachments)
{
  if (cur_number_of_attachments <= 0 || cur_number_of_attachments > GetMaxColorAttachments())
    cur_number_of_attachments = GetMaxColorAttachments();

  glGenFramebuffers(1, &m_id);
  Bind();
  Unbind();
}

LayeredFrameBufferObject::~LayeredFrameBufferObject ()
{
  DestroyAttachments();
  glDeleteFramebuffers(1, &m_id);

  gl::ExitOnGLError("Error when destroying gl::LayeredFrameBufferObject!");
}

void LayeredFrameBufferObject::SetRenderTargetColoAttachment (GLuint color_attach_id, GLuint texture_id)
{
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attach_id, m_color_attachments[texture_id], 0);
}

GLuint LayeredFrameBufferObject::GetColorAttachmentID(int clr_attach_id)
{
  return m_color_attachments[clr_attach_id];
}

bool LayeredFrameBufferObject::GenerateAttachments(unsigned int screen_width, unsigned int screen_height)
{
  width = screen_width;
  height = screen_height;

  Bind();
  for (int i = 0; i < GetCurrentNumberOfAttachments(); i++)
  {
    glGenTextures(1, &m_color_attachments[i]);
    glBindTexture(GL_TEXTURE_2D, m_color_attachments[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  }

  for (int i = 0; i < GetCurrentNumberOfAttachments(); i++)
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, m_color_attachments[i], 0);

  Bind();

  bool ret;
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "There is a problem with the Frame Buffer Object." << std::endl;
    ret = false;
  }
  else
  {
#ifdef _DEBUG
    std::cout << "Frame Buffer Object created." << std::endl;
#endif
    ret = true;
  }

  Unbind();
  return ret;
}

void LayeredFrameBufferObject::DestroyAttachments ()
{
  for (int i = 0; i < GetCurrentNumberOfAttachments(); i++)
    glDeleteTextures(1, &m_color_attachments[i]);
}

bool LayeredFrameBufferObject::Resize (unsigned int screen_width, unsigned int screen_height)
{
  if (screen_width == width && screen_height == height)
    return false;

  Bind();
  DestroyAttachments();
  GenerateAttachments(screen_width, screen_height);

  return true;
}

void LayeredFrameBufferObject::RenderColorAttachments(unsigned int screen_width, unsigned int screen_height)
{
  // output frame = screen
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  // input frame = gBuffer
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);

  GLint yu0 = (GLint)screen_height / 2;
  GLint yu1 = (GLint)screen_height;

  GLint yd0 = (GLint)0;
  GLint yd1 = (GLint)screen_height / 2;

  GLint x00 = (GLint)0;
  GLint x01 = (GLint)screen_width / 4;

  GLint x10 = (GLint)screen_width / 4;
  GLint x11 = (GLint)(screen_width * 2) / 4;

  GLint x20 = (GLint)(screen_width * 2) / 4;
  GLint x21 = (GLint)(screen_width * 3) / 4;

  GLint x30 = (GLint)(screen_width * 3) / 4;
  GLint x31 = (GLint)screen_width;


  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, width, height, x00, yu0, x01, yu1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glBlitFramebuffer(0, 0, width, height, x10, yu0, x11, yu1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT2);
  glBlitFramebuffer(0, 0, width, height, x20, yu0, x21, yu1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT3);
  glBlitFramebuffer(0, 0, width, height, x30, yu0, x31, yu1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT4);
  glBlitFramebuffer(0, 0, width, height, x00, yd0, x01, yd1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT5);
  glBlitFramebuffer(0, 0, width, height, x10, yd0, x11, yd1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT6);
  glBlitFramebuffer(0, 0, width, height, x20, yd0, x21, yd1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT7);
  glBlitFramebuffer(0, 0, width, height, x30, yd0, x31, yd1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void LayeredFrameBufferObject::RenderColorAttachment(unsigned int screen_width, unsigned int screen_height, int id)
{
  //output frame = screen
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  //input frame = framebuffer
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);

  glReadBuffer(GL_COLOR_ATTACHMENT0 + id);
  glBlitFramebuffer(0, 0, width, height, 0, 0, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  // https://stackoverflow.com/questions/11315534/copying-depth-render-buffer-to-the-depth-buffer
  //glBlitFramebuffer(0, 0, width, height, 0, 0, screen_width, screen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

GLint LayeredFrameBufferObject::GetMaxLayers()
{
  GLint res;
  glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &res);
  return res;
}

GLint LayeredFrameBufferObject::GetMaxSamples()
{
  GLint res;
  glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &res);
  return res;
}

GLint LayeredFrameBufferObject::GetFramebufferWidth()
{
  GLint res;
  glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &res);
  return res;
}

GLint LayeredFrameBufferObject::GetFramebufferHeight()
{
  GLint res;
  glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &res);
  return res;
}

/*! Get the Max number of color attachments.
\return number of color attachments.
*/
GLint LayeredFrameBufferObject::GetMaxColorAttachments()
{
  GLint maxcolorattachments;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxcolorattachments);
  return maxcolorattachments;
}

void LayeredFrameBufferObject::PrintFramebufferLimits()
{
  int res;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &res);
  printf("lqc: Max Color Attachments on Framebuffer: %d\n", res);

  glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &res);
  printf("lqc: Max Framebuffer Width: %d\n", res);

  glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &res);
  printf("lqc: Max Framebuffer Height: %d\n", res);

  glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &res);
  printf("lqc: Max Framebuffer Samples: %d\n", res);

  glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &res);
  printf("lqc: Max Framebuffer Layers: %d\n", res);

}

void LayeredFrameBufferObject::PrintFramebufferInfo(GLenum target, GLuint fbo)
{
  int res, i = 0;
  GLint buffer;

  glBindFramebuffer(target, fbo);

  do {
    glGetIntegerv(GL_DRAW_BUFFER0 + i, &buffer);

    if (buffer != GL_NONE) {

      printf("lqc: Shader Output Location %d - color attachment %d\n",
        i, buffer - GL_COLOR_ATTACHMENT0);

      glGetFramebufferAttachmentParameteriv(target, buffer,
        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &res);
      printf("\tAttachment Type: %s\n",
        res == GL_TEXTURE ? "Texture" : "Render Buffer");
      glGetFramebufferAttachmentParameteriv(target, buffer,
        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &res);
      printf("\tAttachment object name: %d\n", res);
    }
    ++i;

  } while (buffer != GL_NONE);
}

GLuint LayeredFrameBufferObject::GetCurrentNumberOfAttachments()
{
  return cur_number_of_attachments;
}