#ifndef WE_VAO_H
#define WE_VAO_H

#include <cstdint>
#include <cmath>
#include <array>

#include <glad/glad.h>

namespace we
{
  class vao
  {
  public:
    struct vertex
    {
      std::float_t position[4];
      std::float_t uv[4];
    };

  public:
    inline static const std::array<vertex, 4> s_rect_vertices
    {
      vertex{ -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
      vertex{ -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
      vertex{  1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f },
      vertex{  1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    };
    inline static const std::array<std::uint32_t, 6> s_rect_elements
    {
      0, 1, 2,
      2, 3, 1,
    };

  public:
    vao() = delete;

  public:
    static void create(std::uint32_t& vao, std::uint32_t vertex_count, const vertex* vertices, std::uint32_t element_count, const std::uint32_t* elements);
  };
}

#endif