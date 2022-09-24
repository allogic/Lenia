#include <system.h>
#include <texture.h>
#include <shader.h>
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
    texture::create_fill(m_textures[e_tex_back], m_system_width, m_system_height, 0.0f);
    texture::create_random_rgb(m_textures[e_tex_gen], m_generator_width, m_generator_height, 0.0f, 1.0f);

    // Create framebuffers
    std::uint32_t attachments[]{ GL_COLOR_ATTACHMENT0 };
    glGenFramebuffers(3, &m_fbos[0]);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[e_fb_front]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[e_tex_front], 0);
    glDrawBuffers(1, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[e_fb_back]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[e_tex_back], 0);
    glDrawBuffers(1, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[e_fb_gen]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[e_tex_gen], 0);
    glDrawBuffers(1, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create buffers
    glGenBuffers(2, &m_buffers[0]);

    // Create vaos
    glGenVertexArrays(1, &m_vaos[e_vao_rect]);

    glBindVertexArray(m_vaos[e_vao_rect]);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[e_buf_vertex]);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vertex), s_rect_vertices, GL_STATIC_READ | GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(0));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(std::float_t) * 4));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[e_buf_element]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(std::uint32_t), s_rect_elements, GL_STATIC_READ | GL_STATIC_DRAW);

    glBindVertexArray(0);

    // Add convolution layers
    m_convolutions.emplace_back(convolution{ "C0", 11, 14.0f, 74.0f, 4.0f, { 0.01f, 0.05f, 0.05f }, { 2.0f, 2.0f, 2.0f }, { 2.0f, 2.0f, 2.0f }, { 2.0f, 2.0f, 2.0f }, { 2.0f, 2.0f, 2.0f } });
    m_convolutions.emplace_back(convolution{ "C1", 21, 3.0f, 74.0f, 4.0f, { 0.05f, 0.01f, 0.05f }, { 2.0f, 2.0f, 2.0f }, { 2.0f, 2.0f, 2.0f }, { 2.0f, 2.0f, 2.0f }, { 2.0f, 2.0f, 2.0f } });
    m_convolutions.emplace_back(convolution{ "C2", 31, 1.0f, 100.0f, 4.0f, { 0.05f, 0.05f, 0.01f }, { 2.0f, 2.0f, 2.0f }, { 2.0f, 2.0f, 2.0f }, { 2.0f, 2.0f, 2.0f }, { 2.0f, 2.0f, 2.0f } });
  }

  void system::swap()
  {
    for (const convolution& convolution : m_convolutions)
    {
      // Compute next state
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbos[e_fb_back]);
      
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_textures[e_tex_front]);
      
      convolution.bind(m_system_width, m_system_height);

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
    }

    // Check if system vanished
    m_iteration++;
    //if (m_iteration % 50 == 0)
    //{
    //  m_iteration = 0;
    //
    //  fill_pseudo_random(0.0, 1.0f);
    //  create_randomized_shader();
    //
    //  //std::float_t avg{ 0.0f };
    //  //compute_color_avg(avg);
    //  //if (avg < 0.1f)
    //  //{
    //  //  
    //  //}
    //}
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
    for (convolution& convolution : m_convolutions)
    {
      convolution.ui();
    }
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