#include <sstream>
#include <format>
#include <cfenv>
#include <random>

#include <system.h>
#include <texture.h>
#include <shader.h>
#include <framebuffer.h>
#include <vao.h>

#include <glad/glad.h>

#include <imgui/imgui.h>

namespace we
{
  system::system(std::uint32_t system_width, std::uint32_t system_height, std::uint32_t generator_width, std::uint32_t generator_height)
    : m_system_width{ system_width }
    , m_system_height{ system_height }
    , m_generator_width{ generator_width }
    , m_generator_height{ generator_height }
  {
    // Create textures
    texture::create_random_rgb(m_textures[e_tex_front], m_system_width, m_system_height, 0.0f, 1.0f);
    //texture::create_from_file(m_textures[e_tex_front], m_system_width, m_system_height, PATTERN_DIR "smile.tga");
    texture::create_fill(m_textures[e_tex_back], m_system_width, m_system_height, 0.0f);
    texture::create_random_rgb(m_textures[e_tex_gen], m_generator_width, m_generator_height, 0.0f, 1.0f);

    // Create framebuffers
    framebuffer::create(m_fbos[e_fb_front], m_textures[e_tex_front]);
    framebuffer::create(m_fbos[e_fb_back], m_textures[e_tex_back]);
    framebuffer::create(m_fbos[e_fb_gen], m_textures[e_tex_gen]);

    // Create vaos
    vao::create(m_vaos[e_vao_rect], 4, &vao::s_rect_vertices[0], 6, &vao::s_rect_elements[0]);

    // Add kernels
    m_kernels.emplace(0, kernel{ "r0", 0, 1.0f, 22, 95.546f, 101.467f, 4, growth{ 14.744f, 1.361f, 9.773f, 4 } });
    m_kernels.emplace(0, kernel{ "r1", 0, 1.0f, 14, 27.401f, 201.791f, 10, growth{ 7.503f, 1.056f, 5.234f, 8 } });
    m_kernels.emplace(0, kernel{ "r2", 0, 1.0f, 26, 72.666f, 355.859f, 3, growth{ 14.210f, 0.311f, 2.862f, 1 } });

    m_kernels.emplace(1, kernel{ "g0", 1, 1.0f, 13, 89.537f, 310.026f, 4, growth{ 19.295f, 1.32f, 6.475f, 13 } });
    m_kernels.emplace(1, kernel{ "g1", 1, 1.0f, 26, 33.693f, 199.018f, 2, growth{ 17.667f, 1.678f, 2.314f, 20 } });
    m_kernels.emplace(1, kernel{ "g2", 1, 1.0f, 27, 85.988f, 408.609f, 8, growth{ 18.425f, 0.01f, 8.164f, 14 } });

    m_kernels.emplace(2, kernel{ "b0", 2, 1.0f, 17, 39.609f, 383.556f, 3, growth{ 15.637f, 0.933f, 2.713f, 2 } });
    m_kernels.emplace(2, kernel{ "b1", 2, 1.0f, 7, 74.299f, 70.204f, 7, growth{ 14.115f, 1.752f, 5.816f, 10 } });
    m_kernels.emplace(2, kernel{ "b2", 2, 1.0f, 13, 63.958f, 495.396f, 13, growth{ 2.907f, 1.915f, 2.45f, 1 } });

    // Build initial state
    rebuild_kernel();
    rebuild_shader();
    rebuild_preview();
  }

  void system::update()
  {
    if (m_dirty)
    {
      m_dirty = 0;

      rebuild_shader();
    }
  }

  void system::swap()
  {
    // Set viewport to system size
    glViewport(0, 0, m_system_width, m_system_height);

    // Compute next state
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbos[e_fb_back]);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures[e_tex_front]);
    
    glUseProgram(m_programs[e_prog_conv]);

    glUniform2f(glGetUniformLocation(m_programs[e_prog_conv], "u_texture_size"), static_cast<std::float_t>(m_system_width), static_cast<std::float_t>(m_system_height));

    auto range0{ m_kernels.equal_range(0) };
    auto range1{ m_kernels.equal_range(1) };
    auto range2{ m_kernels.equal_range(2) };

    for (auto it{ range0.first }; it != range0.second; it++) update_uniforms(it->second);
    for (auto it{ range1.first }; it != range1.second; it++) update_uniforms(it->second);
    for (auto it{ range2.first }; it != range2.second; it++) update_uniforms(it->second);

    glBindVertexArray(m_vaos[e_vao_rect]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Copy back to front
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbos[e_fb_back]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbos[e_fb_front]);

    glBlitFramebuffer(0, 0, m_system_width, m_system_height, 0, 0, m_system_width, m_system_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Copy generator to front
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbos[e_fb_gen]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbos[e_fb_front]);
    
    glBlitFramebuffer(0, 0, m_generator_width, m_generator_height, (m_system_width / 2), (m_system_height / 2), (m_system_width / 2) + m_generator_width, (m_system_height / 2) + m_generator_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Check if system vanished
    m_iteration++;
    if (m_iteration % 200 == 0)
    {
      m_iteration = 0;
    }
  }

  void system::draw(std::float_t x, std::float_t y, std::float_t scale_x, std::float_t scale_y)
  {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbos[e_fb_front]);

    std::uint32_t target_pos_x{ static_cast<std::uint32_t>(x) };
    std::uint32_t target_pos_y{ static_cast<std::uint32_t>(y) };
    std::uint32_t target_width{ static_cast<std::uint32_t>(scale_x * m_system_width) };
    std::uint32_t target_height{ static_cast<std::uint32_t>(scale_y * m_system_height) };

    glBlitFramebuffer(0, 0, m_system_width, m_system_height, target_pos_x, target_pos_y, target_pos_x + target_width, target_pos_y + target_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void system::ui()
  {
    auto range0{ m_kernels.equal_range(0) };
    auto range1{ m_kernels.equal_range(1) };
    auto range2{ m_kernels.equal_range(2) };

    for (auto it{ range0.first }; it != range0.second; it++) ui_kernel(it->second);
    for (auto it{ range1.first }; it != range1.second; it++) ui_kernel(it->second);
    for (auto it{ range2.first }; it != range2.second; it++) ui_kernel(it->second);
  }

  void system::randomize()
  {
    std::random_device random{};
    std::mt19937 generator{ random() };

    auto range0{ m_kernels.equal_range(0) };
    auto range1{ m_kernels.equal_range(1) };
    auto range2{ m_kernels.equal_range(2) };

    for (auto it{ range0.first }; it != range0.second; it++) randomize_kernel(it->second, generator);
    for (auto it{ range1.first }; it != range1.second; it++) randomize_kernel(it->second, generator);
    for (auto it{ range2.first }; it != range2.second; it++) randomize_kernel(it->second, generator);

    rebuild_kernel();
    rebuild_shader();
  }

  void system::rebuild_kernel()
  {
    auto range0{ m_kernels.equal_range(0) };
    auto range1{ m_kernels.equal_range(1) };
    auto range2{ m_kernels.equal_range(2) };

    for (auto it{ range0.first }; it != range0.second; it++) compute_kernel(it->second);
    for (auto it{ range1.first }; it != range1.second; it++) compute_kernel(it->second);
    for (auto it{ range2.first }; it != range2.second; it++) compute_kernel(it->second);
  }

  void system::rebuild_shader()
  {
    shader::destroy(m_programs[e_prog_conv]);

    std::stringstream shader{};

    shader << "#version 460 core\n\n";
    shader << "layout (location = 0) in Forward\n{\n  vec4 uv;\n} i_fwd;\n\n";
    shader << "layout (location = 0) out vec4 o_color;\n\n";
    shader << "layout (location = 0) uniform sampler2D u_texture;\n";
    shader << "layout (location = 1) uniform vec2 u_texture_size;\n\n";

    std::uint32_t location{ 2 };
    {
      auto range0{ m_kernels.equal_range(0) };
      auto range1{ m_kernels.equal_range(1) };
      auto range2{ m_kernels.equal_range(2) };

      for (auto it{ range0.first }; it != range0.second; it++) stringify_uniforms(it->second, shader, location);
      for (auto it{ range1.first }; it != range1.second; it++) stringify_uniforms(it->second, shader, location);
      for (auto it{ range2.first }; it != range2.second; it++) stringify_uniforms(it->second, shader, location);
    }

    {
      auto range0{ m_kernels.equal_range(0) };
      auto range1{ m_kernels.equal_range(1) };
      auto range2{ m_kernels.equal_range(2) };

      for (auto it{ range0.first }; it != range0.second; it++) stringify_kernel(it->second, shader);
      for (auto it{ range1.first }; it != range1.second; it++) stringify_kernel(it->second, shader);
      for (auto it{ range2.first }; it != range2.second; it++) stringify_kernel(it->second, shader);
    }

    {
      auto range0{ m_kernels.equal_range(0) };
      auto range1{ m_kernels.equal_range(1) };
      auto range2{ m_kernels.equal_range(2) };

      for (auto it{ range0.first }; it != range0.second; it++) stringify_growth(it->second, shader);
      for (auto it{ range1.first }; it != range1.second; it++) stringify_growth(it->second, shader);
      for (auto it{ range2.first }; it != range2.second; it++) stringify_growth(it->second, shader);
    }

    shader << "void main()\n{\n";
    shader << "  float fx = 1.0 / u_texture_size.x;\n";
    shader << "  float fy = 1.0 / u_texture_size.y;\n\n";

    {
      auto range0{ m_kernels.equal_range(0) };
      auto range1{ m_kernels.equal_range(1) };
      auto range2{ m_kernels.equal_range(2) };

      for (auto it{ range0.first }; it != range0.second; it++) shader << "  float " << it->second.name << "_sum = 0.0;\n";
      for (auto it{ range1.first }; it != range1.second; it++) shader << "  float " << it->second.name << "_sum = 0.0;\n";
      for (auto it{ range2.first }; it != range2.second; it++) shader << "  float " << it->second.name << "_sum = 0.0;\n";
    }

    shader << "\n";

    {
      auto range0{ m_kernels.equal_range(0) };
      auto range1{ m_kernels.equal_range(1) };
      auto range2{ m_kernels.equal_range(2) };

      for (auto it{ range0.first }; it != range0.second; it++) stringify_convolution(it->second, shader);
      for (auto it{ range1.first }; it != range1.second; it++) stringify_convolution(it->second, shader);
      for (auto it{ range2.first }; it != range2.second; it++) stringify_convolution(it->second, shader);
    }

    shader << "  vec3 c = texture(u_texture, i_fwd.uv.xy).rgb;\n\n";

    {
      auto range0{ m_kernels.equal_range(0) };
      auto range1{ m_kernels.equal_range(1) };
      auto range2{ m_kernels.equal_range(2) };

      for (auto it{ range0.first }; it != range0.second; it++) stringify_results(it->second, shader);
      for (auto it{ range1.first }; it != range1.second; it++) stringify_results(it->second, shader);
      for (auto it{ range2.first }; it != range2.second; it++) stringify_results(it->second, shader);
    }

    //shader << "  float rm = c.r + r0_c + r1_c + r2_c + g0_c + g1_c + g2_c + b0_c + b1_c + b2_c;\n";
    //shader << "  float gm = c.g + r0_c + r1_c + r2_c + g0_c + g1_c + g2_c + b0_c + b1_c + b2_c;\n";
    //shader << "  float bm = c.b + r0_c + r1_c + r2_c + g0_c + g1_c + g2_c + b0_c + b1_c + b2_c;\n\n";

    //shader << "  float rm = c.r + g0_c + g1_c + g2_c + b0_c + b1_c + b2_c;\n";
    //shader << "  float gm = c.g + r0_c + r1_c + r2_c + b0_c + b1_c + b2_c;\n";
    //shader << "  float bm = c.b + r0_c + r1_c + r2_c + g0_c + g1_c + g2_c;\n\n";

    //shader << "  float rm = c.r + g0_c + g1_c + g2_c;\n";
    //shader << "  float gm = c.g + b0_c + b1_c + b2_c;\n";
    //shader << "  float bm = c.b + r0_c + r1_c + r2_c;\n\n";

    shader << "  float rm = c.r + r0_c + g1_c + b2_c;\n";
    shader << "  float gm = c.g + r1_c + g2_c + b0_c;\n";
    shader << "  float bm = c.b + r2_c + g0_c + b1_c;\n\n";

    shader << "  float r = clamp(rm, 0.0, 1.0);\n";
    shader << "  float g = clamp(gm, 0.0, 1.0);\n";
    shader << "  float b = clamp(bm, 0.0, 1.0);\n\n";

    shader << "  o_color = vec4(r, g, b, 1.0);\n";
    shader << "}";

    std::printf(shader.str().c_str());

    shader::create(m_programs[e_prog_conv], shader::s_rect_vertex_source, shader.str());
  }

  void system::rebuild_preview()
  {
    auto range0{ m_kernels.equal_range(0) };
    auto range1{ m_kernels.equal_range(1) };
    auto range2{ m_kernels.equal_range(2) };

    for (auto it{ range0.first }; it != range0.second; it++) texture::create_from_values(it->second.texture, it->second.size, it->second.size, it->second.values);
    for (auto it{ range1.first }; it != range1.second; it++) texture::create_from_values(it->second.texture, it->second.size, it->second.size, it->second.values);
    for (auto it{ range2.first }; it != range2.second; it++) texture::create_from_values(it->second.texture, it->second.size, it->second.size, it->second.values);
  }

  void system::update_uniforms(kernel& kernel)
  {
    glUniform1f(glGetUniformLocation(m_programs[e_prog_conv], std::format("u_{}_time", kernel.name).c_str()), kernel.time);
    glUniform1f(glGetUniformLocation(m_programs[e_prog_conv], std::format("u_{}_growth_height", kernel.name).c_str()), kernel.growth.height);
    glUniform1f(glGetUniformLocation(m_programs[e_prog_conv], std::format("u_{}_growth_offset", kernel.name).c_str()), kernel.growth.offset);
    glUniform1f(glGetUniformLocation(m_programs[e_prog_conv], std::format("u_{}_growth_smoothness", kernel.name).c_str()), kernel.growth.smoothness);
    glUniform1i(glGetUniformLocation(m_programs[e_prog_conv], std::format("u_{}_growth_sharpness", kernel.name).c_str()), kernel.growth.sharpness);
  }

  void system::ui_kernel(kernel& kernel)
  {
    ImGui::PushID(&kernel);
    if (ImGui::CollapsingHeader(kernel.name.c_str()))
    {
      ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());

      ImGui::DragFloat("##Time", &kernel.time, 0.01f, 0.0f, 1.0f, "Time %.3f");

      ImGui::Separator();

      if (ImGui::DragInt("##Kernel Size", reinterpret_cast<std::int32_t*>(&kernel.size), 1.0f, 1, 50, "Kernel Size %d")) m_dirty = 1;
      if (ImGui::DragFloat("##Kernel Offset", &kernel.offset, 0.1f, 0.0f, 0.0f, "Kernel Offset %.3f")) m_dirty = 1;
      if (ImGui::DragFloat("##Kernel Distance", &kernel.distance, 0.1f, 0.0f, 0.0f, "Kernel Distance %.3f")) m_dirty = 1;
      if (ImGui::DragInt("##Kernel Sharpness", reinterpret_cast<std::int32_t*>(&kernel.sharpness), 1.0f, 0, 100, "Kernel Sharpness %d")) m_dirty = 1;

      if (m_dirty)
      {
        texture::destroy(kernel.texture);

        compute_kernel(kernel);

        texture::create_from_values(kernel.texture, kernel.size, kernel.size, kernel.values);
      }

      ImGui::Image(reinterpret_cast<void*>(static_cast<std::uint64_t>(kernel.texture)), { 256.0f, 256.0f });

      ImGui::DragFloat("GrowthHeight", &kernel.growth.height, 0.05f, 0.0f, 50.0f, "Growth Height %.3f");
      ImGui::DragFloat("GrowthOffset", &kernel.growth.offset, 1.0f, 0.0f, 1000.0f, "Growth Offset %.3f");
      ImGui::DragFloat("GrowthSmoothness", &kernel.growth.smoothness, 0.1f, 0.0f, 100.0f, "Growth Smoothness %.3f");
      ImGui::DragInt("GrowthSharpness", reinterpret_cast<std::int32_t*>(&kernel.growth.sharpness), 1.0f, 0, 100, "Growth Sharpness %d");

      static std::array<std::float_t, 64> growth{};
      for (int32_t i = -10; i < 54; i++)
      {
        growth[i + 10] = bump(static_cast<std::float_t>(i), kernel.growth.height, kernel.growth.offset, kernel.growth.smoothness, kernel.growth.sharpness);
      }
      ImGui::PlotLines("", &growth[0], 64, 0, "Growth", -2.0f, 2.0f, { 256.0f, 100.0f });

      ImGui::PopItemWidth();
    }
    ImGui::PopID();
  }

  void system::compute_kernel(kernel& kernel)
  {
    kernel.values.resize(kernel.size * kernel.size * 4);

    for (std::uint32_t i{}; i < kernel.size; i++)
    {
      for (std::uint32_t j{}; j < kernel.size; j++)
      {
        std::uint32_t idx{ (i + j * kernel.size) * 4 };

        std::float_t h{ static_cast<std::float_t>(kernel.size) / 2.0f };
        std::float_t x{ static_cast<std::float_t>(i) - (h - 0.5f) };
        std::float_t y{ static_cast<std::float_t>(j) - (h - 0.5f) };
        std::float_t l{ std::sqrtf(x * x + y * y) };
        std::float_t m{ l + kernel.offset };
        std::float_t s{ std::sinf((m * m) / kernel.distance) };
        std::float_t v{ std::powf(s, static_cast<std::float_t>(kernel.sharpness)) };

        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;

        if (std::fetestexcept(FE_DIVBYZERO)) v = 0.0f;
        if (std::fetestexcept(FE_INVALID)) v = 0.0f;
        if (std::fetestexcept(FE_OVERFLOW)) v = 0.0f;
        if (std::fetestexcept(FE_UNDERFLOW)) v = 0.0f;

        std::feclearexcept(FE_ALL_EXCEPT);

        kernel.values[idx + 0] = v;
        kernel.values[idx + 1] = v;
        kernel.values[idx + 2] = v;
        kernel.values[idx + 3] = 1.0f;
      }
    }
  }

  void system::randomize_kernel(kernel& kernel, std::mt19937& generator)
  {
    std::uniform_int_distribution<std::uint32_t> kernel_dist{ 3, 30 };
    std::uniform_real_distribution<std::float_t> kernel_offset_dist{ 0.0f, 100.0f };
    std::uniform_real_distribution<std::float_t> kernel_distance_dist{ 50.0f, 500.0f };
    std::uniform_int_distribution<std::uint32_t> kernel_sharpness_dist{ 1, 20 };
    
    kernel.size = kernel_dist(generator);
    kernel.offset = kernel_offset_dist(generator);
    kernel.distance = kernel_distance_dist(generator);
    kernel.sharpness = kernel_sharpness_dist(generator);

    std::uniform_real_distribution<std::float_t> growth_height_dist{ 0.0f, 20.0f };
    std::uniform_real_distribution<std::float_t> growth_offset_dist{ 0.0f, 2.0f };
    std::uniform_real_distribution<std::float_t> growth_smoothness_dist{ 0.0f, 10.0f };
    std::uniform_int_distribution<std::uint32_t> growth_sharpness_dist{ 1, 20 };
    
    kernel.growth.height = growth_height_dist(generator);
    kernel.growth.offset = growth_offset_dist(generator);
    kernel.growth.smoothness = growth_smoothness_dist(generator);
    kernel.growth.sharpness = growth_sharpness_dist(generator);
  }

  void system::stringify_uniforms(const kernel& kernel, std::stringstream& shader, std::uint32_t& location)
  {
    shader << "layout (location = " << location++ << ") uniform float u_" << kernel.name << "_time;\n";
    shader << "layout (location = " << location++ << ") uniform float u_" << kernel.name << "_growth_height;\n";
    shader << "layout (location = " << location++ << ") uniform float u_" << kernel.name << "_growth_offset;\n";
    shader << "layout (location = " << location++ << ") uniform float u_" << kernel.name << "_growth_smoothness;\n";
    shader << "layout (location = " << location++ << ") uniform int u_" << kernel.name << "_growth_sharpness;\n\n";
  }

  void system::stringify_kernel(const kernel& kernel, std::stringstream& shader)
  {
    shader << "const float c_" << kernel.name << "_kernel[" << kernel.size << "][" << kernel.size << "] =\n{\n";
    for (std::uint32_t i{}; i < kernel.size; i++)
    {
      shader << "  { ";
      for (std::uint32_t j{}; j < kernel.size; j++)
      {
        std::uint32_t idx{ (i + j * kernel.size) * 4 };
        shader << std::format("{:.7f}", kernel.values[idx]) << ", ";
      }
      shader << "},\n";
    }
    shader << "};\n\n";
  }

  void system::stringify_growth(const kernel& kernel, std::stringstream& shader)
  {
    shader << "float " << kernel.name << "_growth(float x)\n{\n";
    shader << "  return (u_" << kernel.name << "_growth_height / (1.0 + pow(abs((x - u_" << kernel.name << "_growth_offset) / u_" << kernel.name << "_growth_smoothness), u_" << kernel.name << "_growth_sharpness))) - 1.0;\n";
    shader << "}\n\n";
  }

  void system::stringify_convolution(const kernel& kernel, std::stringstream& shader)
  {
    std::float_t kernel_half_size{ static_cast<std::float_t>(kernel.size) / 2 };

    shader << "  for (int i = 0; i < " << kernel.size << "; i++)\n  {\n";
    shader << "    for (int j = 0; j < " << kernel.size << "; j++)\n    {\n";
    shader << "      float u = fx * (float(i) - " << kernel_half_size << ");\n";
    shader << "      float v = fy * (float(j) - " << kernel_half_size << ");\n";

    switch (kernel.channel)
    {
      case 0: shader << "      " << kernel.name << "_sum += c_" << kernel.name << "_kernel[i][j] * texture(u_texture, i_fwd.uv.xy + vec2(u, v)).r;\n"; break;
      case 1: shader << "      " << kernel.name << "_sum += c_" << kernel.name << "_kernel[i][j] * texture(u_texture, i_fwd.uv.xy + vec2(u, v)).g;\n"; break;
      case 2: shader << "      " << kernel.name << "_sum += c_" << kernel.name << "_kernel[i][j] * texture(u_texture, i_fwd.uv.xy + vec2(u, v)).b;\n"; break;
    }

    shader << "    }\n";
    shader << "  }\n\n";
  }

  void system::stringify_results(const kernel& kernel, std::stringstream& shader)
  {
    shader << "  float " << kernel.name << "_g = " << kernel.name << "_growth(" << kernel.name << "_sum);\n";
    shader << "  float " << kernel.name << "_avg = " << kernel.name << "_sum / " << kernel.size * kernel.size << ";\n";
    shader << "  float " << kernel.name << "_c = u_" << kernel.name << "_time * " << kernel.name << "_avg / " << kernel.name << "_g;\n\n";
  }

  std::float_t system::bump(std::float_t x, std::float_t height, std::float_t offset, std::float_t smoothness, std::uint32_t sharpness)
  {
    return (height / (1.0f + std::powf(std::fabsf((x - offset) / smoothness), static_cast<std::float_t>(sharpness)))) - 1.0f;
  }

  //void system::compute_color_avg(std::float_t& avg)
  //{
  //  std::vector<std::float_t> values{};
  //  std::float_t sum{};
  //  std::uint32_t size{ m_system_width * m_system_height * 4 };
  //
  //  values.resize(size);
  //
  //  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbos[0]);
  //  glReadPixels(0, 0, m_system_width, m_system_height, GL_RGBA, GL_FLOAT, &values[0]);
  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  //
  //  for (std::uint32_t i{}; i < size; i += 4)
  //  {
  //    sum += values[i + 0];
  //  }
  //
  //  avg = sum / (m_system_width * m_system_height);
  //}
}