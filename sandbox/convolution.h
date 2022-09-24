#ifndef WE_CONVOLUTION_H
#define WE_CONVOLUTION_H

#include <cstdint>
#include <cmath>
#include <array>
#include <vector>
#include <random>
#include <sstream>

namespace we
{
  class convolution
  {
  public:
    enum texture_idx
    {
      e_tex_conv,
    };
    enum shader_idx
    {
      e_prog_conv,
    };

  public:
    convolution(
      const std::string& name,
      std::uint32_t kernel_size,
      std::float_t kernel_offset,
      std::float_t kernel_distance,
      std::float_t kernel_sharpness,
      const std::array<std::float_t, 3>& time_delta_fixed,
      const std::array<std::float_t, 3>& growth_height,
      const std::array<std::float_t, 3>& growth_offset,
      const std::array<std::float_t, 3>& growth_smoothness,
      const std::array<std::float_t, 3>& growth_sharpness);

  public:
    void bind(std::uint32_t system_width, std::uint32_t system_height) const;
    void ui();

    void create_kernel();
    void create_shader();

  private:
    std::string m_name{};

    std::uint32_t m_kernel_size{ 15 };
    std::float_t m_kernel_offset{ 14.0f };
    std::float_t m_kernel_distance{ 74.0f };
    std::float_t m_kernel_sharpness{ 4.0f };

    std::array<std::float_t, 3> m_time_delta_fixed{ 0.05f, 0.05f, 0.05f };

    std::array<std::float_t, 3> m_growth_height{ 2.0f, 2.0f, 2.0f };
    std::array<std::float_t, 3> m_growth_offset{ 2.0f, 2.0f, 2.0f };
    std::array<std::float_t, 3> m_growth_smoothness{ 2.0f, 2.0f, 2.0f };
    std::array<std::float_t, 3> m_growth_sharpness{ 2.0f, 2.0f, 2.0f };

    //std::random_device m_random{};
    //std::mt19937 m_generator{ m_random() };

    std::vector<std::float_t> m_kernel{};

    std::uint32_t m_textures[1]{};
    std::uint32_t m_programs[1]{};
  };
}

#endif