#ifndef WE_TEXTURE_H
#define WE_TEXTURE_H

#include <cstdint>
#include <string>
#include <vector>

namespace we
{
  class texture
  {
  public:
    texture() = delete;

  public:
    static void create_fill(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, std::float_t value);
    static void create_random_r(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, std::float_t min, std::float_t max);
    static void create_random_rgb(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, std::float_t min, std::float_t max);
    static void create_from_file(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, const std::string& file);
    static void create_from_values(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, const std::vector<std::float_t>& values);

    static void destroy(std::uint32_t texture);
  };
}

#endif