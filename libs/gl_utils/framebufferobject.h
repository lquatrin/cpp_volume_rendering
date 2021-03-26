/*!
\file GBuffer.h
\brief Framebuffer class.
\author Leonardo Quatrin Campagnolo
*/
#ifndef GL_UTILS_FRAMEBUFFER_OBJECT_H
#define GL_UTILS_FRAMEBUFFER_OBJECT_H

#include <GL/glew.h>

/*! Class to store a framebuffer used in GLSLVisualization.
*/
namespace gl
{
  class FrameBufferObject
  {
  public:
    /*! Unbind Framebuffer Object (bind GL_FRAMEBUFFER to 0).
    */
    static void Unbind ();
    
    /*! Bind Framebuffer Object to m_id.
    */
    void Bind ();
    
    /*! Constructor
    \param width width of the new framebuffer.
    \param height height of the new framebuffer.
    */
    FrameBufferObject (GLuint custom_number_of_attachments = 0, bool use_depth_buffer = true, int _bits = 32);

    /*! Destructor
    */
    ~FrameBufferObject ();

    GLuint GetColorAttachmentID (int clr_attach_id);

    /*! Generate Color Attachments and Depth Attachment
    */
    bool GenerateAttachments (unsigned int screen_width, unsigned int screen_height);
    
    /*! Destroy all Color Attachments and Depth Attachment
    */
    void DestroyAttachments ();

    /*! Delete the current framebuffer and create another with the param size.
    \param width width of the new framebuffer.
    \param height height of the new framebuffer.
    */
    bool Resize (unsigned int screen_width, unsigned int screen_height);

    /*! Render all color attachments.
    \param width width value of the area available to be rendered.
    \param height height value of the area available to be rendered.
    */
    void RenderColorAttachments (unsigned int screen_width, unsigned int screen_height);
    
    /*! Render just one color attachment.
    \param width width value of the area available to be rendered.
    \param height height value of the area available to be rendered.
    \param id index of the color attachment.
    */
    void RenderColorAttachment (unsigned int screen_width, unsigned int screen_height, int id);

    /*! Return the max number of layers.
    */
    GLint GetMaxLayers ();

    /*! Return the max number of samples.
    */
    GLint GetMaxSamples ();

    /*! Get the framebuffer width.
    \return FrameBuffer width.
    */
    GLint GetFramebufferWidth ();
    
    /*! Get the framebuffer height.
    \return FrameBuffer height.
    */
    GLint GetFramebufferHeight ();

    /*! Get the Max number of color attachments.
    \return number of color attachments.
    */
    GLint GetMaxColorAttachments ();

    /*! Print some FrameBuffer limits (GL_MAX_COLOR_ATTACHMENTS, GL_MAX_FRAMEBUFFER_WIDTH, GL_MAX_FRAMEBUFFER_HEIGHT, GL_MAX_FRAMEBUFFER_SAMPLES, GL_MAX_FRAMEBUFFER_LAYERS)
    */
    void PrintFramebufferLimits();

    /*! Print a FrameBuffer Info.
    */
    void PrintFramebufferInfo(GLenum target, GLuint fbo);

    GLuint GetCurrentNumberOfAttachments ();

    GLuint GetID ()
    {
      return m_id;
    }

  protected:
  private:
    GLuint cur_number_of_attachments;
    bool use_depth_buffer;
    int bits;

    GLuint m_id;
    GLuint m_color_attachments[GL_MAX_COLOR_ATTACHMENTS];
    GLuint m_depth_attachment;

    unsigned int width;
    unsigned int height;
  };
}

#endif