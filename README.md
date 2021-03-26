## C++ Volume Rendering

A volume rendering application with different techniques, trying to be as similar as possible to the results from each paper. Any contribution is welcome!

### Project settings

---

* Developed using Visual Studio cmake extension (2017, 2019).

* Using C++ and GLSL

* Uses glew, freeglut/glfw and glm

* Supported Volumes: .raw, .pvm and .syn

* Supported Transfer Functions: 1D Piecewise linear .tf1d


### Implemented methods

---

* 1-Pass - Ray Casting
  - Structured Single-pass GLSL Ray Casting

* 1-Pass - Ray Casting - Cone Ground Truth (Steps)
  - Structured Single-pass Ray Casting: Directional Ambient Occlusion and Shadows with Cone Tracing
  - Ray Tracing Ground Truth
  - Implements: 
    - Directional Ambient Occlusion
    - Cone Shadows

* 1-Pass - Ray Casting - Dir. Occlusion Shading
   - Structured Single-pass Ray Casting: Directional Ambient Occlusion and Shadows with Cone Tracing
   - Leonardo Q. Campagnolo, Waldemar Celes. Interactive directional ambient occlusion and shadow computations for volume ray casting. Computers & Graphics, Volume 84, 2019, Pages 66-76, ISSN 0097-8493. doi: 10.1016/j.cag.2019.08.009.
   - Links: [Elsevier](https://www.sciencedirect.com/science/article/abs/pii/S0097849319301372)
