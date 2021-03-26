#ifndef GL_UTILS_SPHERE_H
#define GL_UTILS_SPHERE_H

#include <vector>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gl
{
  class Sphere
  {
  public:
    // r = [0, inf)
    // t = [0, pi]
    // p = [0, 2pi]
    Sphere(int N, float r)
    {
      int NSAMPLES = N - 2;
      std::vector<GLuint> sphere_triangles;

      for (int i = 0; i < NSAMPLES - 1; i++)
      {
        for (int j = 0; j < NSAMPLES; j++)
        {
          float t = (float(i + 1) / float(NSAMPLES)) * glm::pi<float>();
          float p = (float(j) / float(NSAMPLES)) * 2.0f * glm::pi<float>();

          sphere_points.push_back(glm::vec3(
            r * sin(t) * cos(p),
            r * sin(t) * sin(p),
            r * cos(t)
            ));

          if (i < NSAMPLES - 2)
          {
            int i_p1 = (i + 1) % NSAMPLES;

            int j_n1 = (j - 1) < 0 ? NSAMPLES - 1 : j - 1;
            int j_p1 = (j + 1) % NSAMPLES;

            sphere_triangles.push_back(i    * NSAMPLES + j);
            sphere_triangles.push_back(i_p1 * NSAMPLES + j);
            sphere_triangles.push_back(i    * NSAMPLES + j_p1);

            sphere_triangles.push_back(i    * NSAMPLES + j);
            sphere_triangles.push_back(i_p1 * NSAMPLES + j_n1);
            sphere_triangles.push_back(i_p1 * NSAMPLES + j);
          }
        }
      }

      // add extreme points:
      {
        sphere_points.push_back(glm::vec3(0.0f, 0.0f, r));
        int flast = sphere_points.size() - 1;
        int ref_i = 0;
        for (int j = 0; j < NSAMPLES; j++)
        {
          int j_n1 = (j - 1) < 0 ? NSAMPLES - 1 : j - 1;
          index_buffer_list.push_back(flast);
          index_buffer_list.push_back(ref_i * NSAMPLES + j);
          index_buffer_list.push_back(ref_i * NSAMPLES + j_n1);
        }
      }

      for (int i = 0; i < sphere_triangles.size(); i++)
      {
        index_buffer_list.push_back(sphere_triangles[i]);
      }

      {
        sphere_points.push_back(glm::vec3(0.0f, 0.0f, -r));
        int llast = sphere_points.size() - 1;
        int ref_i = NSAMPLES - 2;
        for (int j = 0; j < NSAMPLES; j++)
        {
          index_buffer_list.push_back(llast);
          index_buffer_list.push_back(ref_i * NSAMPLES + j);
          index_buffer_list.push_back(ref_i * NSAMPLES + (j + 1) % NSAMPLES);
        }
      }
    }

    ~Sphere() {}

    std::vector<glm::vec3> GetSphereVertices()
    {
      return sphere_points;
    }

    std::vector<GLuint> GetSphereIndexList()
    {
      return index_buffer_list;
    }

    void DrawPoints ()
    {
      glBegin(GL_POINTS);
      for (int i = 0; i < sphere_points.size(); i++)
      {
        glVertex3f(sphere_points[i].x, sphere_points[i].y, sphere_points[i].z);
      }
      glEnd();
    }

    void Draw ()
    {
      glBegin(GL_TRIANGLES);
      for (int i = 0; i < index_buffer_list.size(); i += 3)
      {
        glm::vec3 v1 = sphere_points[index_buffer_list[i]];
        glm::vec3 v2 = sphere_points[index_buffer_list[i + 1]];
        glm::vec3 v3 = sphere_points[index_buffer_list[i + 2]];

        glVertex3f(v1.x, v1.y, v1.z);

        glVertex3f(v2.x, v2.y, v2.z);

        glVertex3f(v3.x, v3.y, v3.z);
      }
      glEnd();

    }

  private:
    std::vector<glm::vec3> sphere_points;
    std::vector<GLuint> index_buffer_list;

  };
}

#endif