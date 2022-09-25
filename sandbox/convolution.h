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
  class system;

  class convolution
  {
  public:
    enum channel_idx
    {
      e_ch_r,
      e_ch_g,
      e_ch_b,
    };
    enum texture_idx
    {
      e_tex_conv,
    };

  public:
    convolution(
      system* system,
      const std::string& name,
      channel_idx channel,
      std::uint32_t kernel_size,
      std::float_t kernel_offset,
      std::float_t kernel_distance,
      std::uint32_t kernel_sharpness,
      std::float_t time_delta_fixed,
      std::float_t growth_height,
      std::float_t growth_offset,
      std::float_t growth_smoothness,
      std::uint32_t growth_sharpness);

  public:
    inline const std::string& get_name() const { return m_name; }
    inline channel_idx get_channel() const { return m_channel; }

    inline std::uint32_t get_kernel_size() const { return m_kernel_size; }
    inline std::float_t get_kernel_offset() const { return m_kernel_offset; }
    inline std::float_t get_kernel_distance() const { return m_kernel_distance; }
    inline std::uint32_t get_kernel_sharpness() const { return m_kernel_sharpness; }
    inline const std::vector<std::float_t>& get_kernel_values() const { return m_kernel_values; }

    inline std::float_t get_time_delta_fixed() const { return m_time_delta_fixed; }

    inline std::float_t get_growth_height() const { return m_growth_height; }
    inline std::float_t get_growth_offset() const { return m_growth_offset; }
    inline std::float_t get_growth_smoothness() const { return m_growth_smoothness; }
    inline std::uint32_t get_growth_sharpness() const { return m_growth_sharpness; }

  public:
    void ui();
    void rebuild_kernel();

  private:
    std::float_t bump(std::float_t x);

  private:
    system* m_system{};
    std::string m_name{};
    channel_idx m_channel{};

    std::uint32_t m_kernel_size{};
    std::float_t m_kernel_offset{};
    std::float_t m_kernel_distance{};
    std::uint32_t m_kernel_sharpness{};
    std::vector<std::float_t> m_kernel_values{};

    std::float_t m_time_delta_fixed{};

    std::float_t m_growth_height{};
    std::float_t m_growth_offset{};
    std::float_t m_growth_smoothness{};
    std::uint32_t m_growth_sharpness{};

    std::array<std::uint32_t, 1> m_textures{};
  };
}

#endif