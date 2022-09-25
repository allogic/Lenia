

#include <convolution.h>
#include <system.h>
#include <texture.h>
#include <shader.h>

#include <glad/glad.h>

#include <imgui/imgui.h>

namespace we
{
  convolution::convolution(
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
    std::uint32_t growth_sharpness)
    : m_system{ system }
    , m_name{ name }
    , m_channel{ channel }
    , m_kernel_size{ kernel_size }
    , m_kernel_offset{ kernel_offset }
    , m_kernel_distance{ kernel_distance }
    , m_kernel_sharpness{ kernel_sharpness }
    , m_time_delta_fixed{ time_delta_fixed }
    , m_growth_height{ growth_height }
    , m_growth_offset{ growth_offset }
    , m_growth_smoothness{ growth_smoothness }
    , m_growth_sharpness{ growth_sharpness }
  {
    // Build first kernel
    rebuild_kernel();

    // Create textures
    texture::create_from_values(m_textures[e_tex_conv], m_kernel_size, m_kernel_size, m_kernel_values);
  }

  void convolution::ui()
  {
    ImGui::Begin(&m_name[0]);

    

    ImGui::End();
  }

  void convolution::rebuild_kernel()
  {
    
  }

  
}