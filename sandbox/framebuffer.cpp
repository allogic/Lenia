#include <array>

#include <framebuffer.h>

#include <glad/glad.h>

namespace we
{
  void framebuffer::create(std::uint32_t& fbo, std::uint32_t attachment0)
  {
    glGenFramebuffers(3, &fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, attachment0, 0);

    std::array<std::uint32_t, 1> attachments{ GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, &attachments[0]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}