#ifndef WE_VAO_H
#define WE_VAO_H

#include <cstdint>
#include <cmath>

namespace we
{
  struct vertex
  {
    std::float_t position[4];
    std::float_t uv[4];
  };

  inline static const vertex s_rect_vertices[]
  {
    { -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f },
    {  1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
  };
  inline static const std::uint32_t s_rect_elements[]
  {
    0, 1, 2,
    2, 3, 1,
  };
}

#endif