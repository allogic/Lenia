#ifndef WE_SYSTEM_H
#define WE_SYSTEM_H

#include <cstdint>
#include <cmath>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>

#define PATTERN_DIR "C:\\Users\\Michael\\Downloads\\Lenia\\patterns\\"

namespace we
{
  struct growth
  {
    std::float_t height{};
    std::float_t offset{};
    std::float_t smoothness{};
    std::uint32_t sharpness{};
  };

  struct kernel
  {
    std::string name{};
    std::uint32_t channel{};
    std::float_t time;
    std::uint32_t size{};
    std::float_t offset{};
    std::float_t distance{};
    std::uint32_t sharpness{};
    growth growth{};
    std::vector<std::float_t> values{};
    std::uint32_t texture{};
  };

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
    enum vao_idx
    {
      e_vao_rect,
    };
    enum shader_idx
    {
      e_prog_conv,
    };

  public:
    system(std::uint32_t system_width, std::uint32_t system_height, std::uint32_t generator_width, std::uint32_t generator_height);

  public:
    inline void set_dirty() { m_dirty = 1; }
    inline std::uint32_t get_dirty() const { return m_dirty; }

  public:
    void update();
    void swap();
    void draw(std::float_t x, std::float_t y, std::float_t scale_x, std::float_t scale_y);
    void ui();
    void randomize();

  private:
    void rebuild_kernel();
    void rebuild_shader();
    void rebuild_preview();

  private:
    void update_uniforms(kernel& kernel);

  private:
    void ui_kernel(kernel& kernel);
    void compute_kernel(kernel& kernel);
    void randomize_kernel(kernel& kernel, std::mt19937& generator);

  private:
    void stringify_uniforms(const kernel& kernel, std::stringstream& shader, std::uint32_t& location);
    void stringify_kernel(const kernel& kernel, std::stringstream& shader);
    void stringify_growth(const kernel& kernel, std::stringstream& shader);
    void stringify_convolution(const kernel& kernel, std::stringstream& shader);
    void stringify_results(const kernel& kernel, std::stringstream& shader);

  private:
    std::float_t bump(std::float_t x, std::float_t height, std::float_t offset, std::float_t smoothness, std::uint32_t sharpness);

  private:
    std::uint32_t m_system_width{};
    std::uint32_t m_system_height{};

    std::uint32_t m_generator_width{};
    std::uint32_t m_generator_height{};

    std::unordered_multimap<std::uint32_t, kernel> m_kernels{};

    std::array<std::uint32_t, 3> m_textures{};
    std::array<std::uint32_t, 3> m_fbos{};
    std::array<std::uint32_t, 1> m_vaos{};
    std::array<std::uint32_t, 1> m_programs{};

    std::uint32_t m_iteration{};
    std::uint32_t m_dirty{};
  };
}

#endif