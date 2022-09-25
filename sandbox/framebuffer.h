#ifndef WE_FRAMEBUFFER_H
#define WE_FRAMEBUFFER_H

#include <cstdint>

namespace we
{
  class framebuffer
  {
  public:
    framebuffer() = delete;

  public:
    static void create(std::uint32_t& fbo, std::uint32_t attachment0);
  };
}

#endif