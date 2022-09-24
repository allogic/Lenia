#ifndef WE_SYSTEM_H
#define WE_SYSTEM_H

#include <cstdint>
#include <cmath>
#include <array>
#include <vector>

#include <convolution.h>

namespace we
{
  class system
  {
  public:
    enum texture_idx
    {
      e_tex_front,
      e_tex_back,
      e_tex_gen,
    };
    enum framebuffer_idx
    {
      e_fb_front,
      e_fb_back,
      e_fb_gen,
    };
    enum buffer_idx
    {
      e_buf_vertex,
      e_buf_element,
    };
    enum vao_idx
    {
      e_vao_rect,
    };

  public:
    system(std::uint32_t system_width, std::uint32_t system_height, std::uint32_t generator_width, std::uint32_t generator_height);

  public:
    void swap();
    void draw(std::float_t x, std::float_t y, std::float_t scale_x, std::float_t scale_y);
    void ui();

    //void compute_color_avg(std::float_t& avg);

  private:
    std::uint32_t m_system_width{};
    std::uint32_t m_system_height{};

    std::uint32_t m_generator_width{};
    std::uint32_t m_generator_height{};

    std::vector<convolution> m_convolutions{};

    std::array<std::uint32_t, 3> m_textures{};
    std::array<std::uint32_t, 3> m_fbos{};
    std::array<std::uint32_t, 2> m_buffers{};
    std::array<std::uint32_t, 1> m_vaos{};

    std::uint32_t m_iteration{};
  };
}

#endif