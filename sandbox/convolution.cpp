#include <cstring>
#include <format>

#include <convolution.h>
#include <texture.h>
#include <shader.h>

#include <glad/glad.h>

#include <imgui/imgui.h>

namespace we
{
  convolution::convolution(
    const std::string& name,
    std::uint32_t kernel_size,
    std::float_t kernel_offset,
    std::float_t kernel_distance,
    std::float_t kernel_sharpness,
    const std::array<std::float_t, 3>& time_delta_fixed,
    const std::array<std::float_t, 3>& growth_height,
    const std::array<std::float_t, 3>& growth_offset,
    const std::array<std::float_t, 3>& growth_smoothness,
    const std::array<std::float_t, 3>& growth_sharpness)
    : m_name{ name }
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
    // Generate initial kernel
    create_kernel();

    // Create textures
    texture::create_from_values(m_textures[e_tex_conv], m_kernel_size, m_kernel_size, m_kernel);

    // Generate initial shader
    create_shader();
  }

  void convolution::bind(std::uint32_t system_width, std::uint32_t system_height) const
  {
    glUseProgram(m_programs[e_prog_conv]);

    glUniform3fv(glGetUniformLocation(m_programs[e_prog_conv], "u_time_delta_fixed"), 1, &m_time_delta_fixed[0]);
    glUniform3fv(glGetUniformLocation(m_programs[e_prog_conv], "u_growth_height"), 1, &m_growth_height[0]);
    glUniform3fv(glGetUniformLocation(m_programs[e_prog_conv], "u_growth_offset"), 1, &m_growth_offset[0]);
    glUniform3fv(glGetUniformLocation(m_programs[e_prog_conv], "u_growth_smoothness"), 1, &m_growth_smoothness[0]);
    glUniform3fv(glGetUniformLocation(m_programs[e_prog_conv], "u_growth_sharpness"), 1, &m_growth_sharpness[0]);
    glUniform2f(glGetUniformLocation(m_programs[e_prog_conv], "u_texture_size"), static_cast<std::float_t>(system_width), static_cast<std::float_t>(system_height));
  }

  void convolution::ui()
  {
    ImGui::Begin(&m_name[0]);

    ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());

    ImGui::DragFloat3("TimeDeltaFixed", &m_time_delta_fixed[0], 0.01f, 0.0f, 1.0f);

    ImGui::Separator();

    static bool is_dirty = false;
    if (ImGui::DragInt("Kernel Size", reinterpret_cast<std::int32_t*>(&m_kernel_size), 1.0f, 1, 100)) is_dirty = true;
    if (ImGui::DragFloat("Kernel Offset", &m_kernel_offset, 1.0f, 0.0f, 0.0f)) is_dirty = true;
    if (ImGui::DragFloat("Kernel Distance", &m_kernel_distance, 1.0f, 0.0f, 0.0f)) is_dirty = true;
    if (ImGui::DragFloat("Kernel Sharpness", &m_kernel_sharpness, 1.0f, 0.0f, 0.0f)) is_dirty = true;

    if (is_dirty)
    {
      texture::destroy(m_textures[e_tex_conv]);

      create_kernel();

      texture::create_from_values(m_textures[e_tex_conv], m_kernel_size, m_kernel_size, m_kernel);
    }

    ImGui::Image(reinterpret_cast<void*>(static_cast<uint64_t>(m_textures[e_tex_conv])), { 256.0f, 256.0f });

    if (ImGui::Button("Generate Kernel", { ImGui::GetWindowContentRegionWidth(), 0.0f }))
    {
      //std::uniform_int_distribution<std::int32_t> kernel_dist{ 15, 15 };
      //std::uniform_real_distribution<std::float_t> kernel_offset_dist{ 14.0f, 14.0f };
      //std::uniform_real_distribution<std::float_t> kernel_distance_dist{ 74.0f, 74.0f };
      //std::uniform_real_distribution<std::float_t> kernel_sharpness_dist{ 4.0f, 4.0f };

      //m_kernel_size = kernel_dist(m_generator);
      //m_kernel_offset = kernel_offset_dist(m_generator);
      //m_kernel_distance = kernel_distance_dist(m_generator);
      //m_kernel_sharpness = kernel_sharpness_dist(m_generator);

      create_shader();
    }

    ImGui::Separator();

    ImGui::DragFloat3("GrowthHeight", &m_growth_height[0], 0.05f, 0.0f, 50.0f);
    ImGui::DragFloat3("GrowthOffset", &m_growth_offset[0], 1.0f, 0.0f, 1000.0f);
    ImGui::DragFloat3("GrowthSmoothness", &m_growth_smoothness[0], 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat3("GrowthSharpness", &m_growth_sharpness[0], 0.1f, 0.0f, 100.0f);

    if (ImGui::Button("Randomize Growth", { ImGui::GetWindowContentRegionWidth(), 0.0f }))
    {
      //std::uniform_real_distribution<std::float_t> growth_height_dist{ 0.0f, 5.0f };
      //std::uniform_real_distribution<std::float_t> growth_offset_dist{ 0.0f, 100.0f };
      //std::uniform_real_distribution<std::float_t> growth_smoothness_dist{ 0.0f, 10.0f };
      //std::uniform_real_distribution<std::float_t> growth_sharpness_dist{ 0.0f, 10.0f };

      //m_growth_height = growth_height_dist(m_generator);
      //m_growth_offset = growth_offset_dist(m_generator);
      //m_growth_smoothness = growth_smoothness_dist(m_generator);
      //m_growth_sharpness = growth_sharpness_dist(m_generator);
    }

    ImGui::PopItemWidth();

    ImGui::End();
  }

  void convolution::create_kernel()
  {
    m_kernel.resize(m_kernel_size * m_kernel_size * 4);

    for (std::uint32_t i{}; i < m_kernel_size; i++)
    {
      for (std::uint32_t j{}; j < m_kernel_size; j++)
      {
        std::uint32_t idx{ (i + j * m_kernel_size) * 4 };

        std::float_t h{ static_cast<std::float_t>(m_kernel_size) / 2.0f };
        std::float_t x{ static_cast<std::float_t>(i) - (h - 0.5f) };
        std::float_t y{ static_cast<std::float_t>(j) - (h - 0.5f) };
        std::float_t l{ std::sqrtf(x * x + y * y) };
        std::float_t m{ l + m_kernel_offset };
        std::float_t s{ std::sinf((m * m) / m_kernel_distance) };
        std::float_t v{ std::powf(s, m_kernel_sharpness) };

        if (l > (static_cast<std::float_t>(m_kernel_size) / 2.0f))
        {
          v = 0.0f;
        }

        m_kernel[idx + 0] = v;
        m_kernel[idx + 1] = v;
        m_kernel[idx + 2] = v;
        m_kernel[idx + 3] = 1.0f;
      }
    }
  }

  void convolution::create_shader()
  {
    shader::destroy(m_programs[e_prog_conv]);

    std::stringstream m_shader{};

    m_shader << "#version 460 core\n\n";
    m_shader << "layout (location = 0) in Forward\n{\n  vec4 uv;\n} i_fwd;\n\n";
    m_shader << "layout (location = 0) out vec4 o_color;\n\n";
    m_shader << "layout (location = 0) uniform sampler2D u_texture;\n";
    m_shader << "layout (location = 1) uniform vec3 u_time_delta_fixed;\n";
    m_shader << "layout (location = 2) uniform vec3 u_growth_height;\n";
    m_shader << "layout (location = 3) uniform vec3 u_growth_offset;\n";
    m_shader << "layout (location = 4) uniform vec3 u_growth_smoothness;\n";
    m_shader << "layout (location = 5) uniform vec3 u_growth_sharpness;\n";
    m_shader << "layout (location = 6) uniform vec2 u_texture_size;\n\n";

    m_shader << "const int c_kernel_size = " << m_kernel_size << ";\n\n";

    m_shader << "const float c_kernel[" << m_kernel_size << "][" << m_kernel_size << "] =\n{\n";
    for (std::uint32_t i{}; i < m_kernel_size; i++)
    {
      m_shader << "  { ";
      for (std::uint32_t j{}; j < m_kernel_size; j++)
      {
        std::uint32_t idx{ (i + j * m_kernel_size) * 4 };
        m_shader << std::format("{:.7f}", m_kernel[idx]) << ", ";
      }
      m_shader << "},\n";
    }
    m_shader << "};\n\n";

    m_shader << "vec3 growth(vec3 x)\n{\n";
    m_shader << "  return (u_growth_height / (vec3(1.0) + pow(abs((x - u_growth_offset) / u_growth_smoothness), u_growth_sharpness))) - vec3(1.0);\n";
    m_shader << "}\n\n";

    m_shader << "void main()\n{\n";
    m_shader << "  vec3 sum = vec3(0.0);\n\n";
    m_shader << "  float fx = 1.0 / u_texture_size.x;\n";
    m_shader << "  float fy = 1.0 / u_texture_size.y;\n\n";
    m_shader << "  for (int i = 0; i < " << m_kernel_size << "; i++)\n  {\n";
    m_shader << "    for (int j = 0; j < " << m_kernel_size << "; j++)\n    {\n";
    m_shader << "      if (i == ceil(c_kernel_size / 2) && j == ceil(c_kernel_size / 2)) continue;\n";
    m_shader << "      float u = fx * (float(i) - " << (static_cast<std::float_t>(m_kernel_size) / 2) << ");\n";
    m_shader << "      float v = fy * (float(j) - " << (static_cast<std::float_t>(m_kernel_size) / 2) << ");\n";
    m_shader << "      sum += c_kernel[i][j] * texture(u_texture, i_fwd.uv.xy + vec2(u, v)).rgb;\n";
    m_shader << "    }\n";
    m_shader << "  }\n\n";
    m_shader << "  vec3 c0 = texture(u_texture, i_fwd.uv.xy).rgb;\n";
    m_shader << "  vec3 g = growth(sum) * u_time_delta_fixed;\n";
    m_shader << "  vec3 c1 = clamp(c0 + g, 0.0, 1.0);\n\n";
    m_shader << "  o_color = vec4(c1, 1.0);\n";
    m_shader << "}";

    shader::create(m_programs[e_prog_conv], shader::s_rect_vertex_source, m_shader.str());
  }
}