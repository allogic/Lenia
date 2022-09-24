#include <string>
#include <vector>
#include <fstream>

#include <shader.h>

#include <glad/glad.h>

namespace we
{
  void shader::create(std::uint32_t& program, const std::string& vertex_source, const std::string& fragment_source)
  {
    program = glCreateProgram();

    std::uint32_t vid{ glCreateShader(GL_VERTEX_SHADER) };
    std::uint32_t fid{ glCreateShader(GL_FRAGMENT_SHADER) };

    const char* vs{ &vertex_source[0] };
    glShaderSource(vid, 1, &vs, nullptr);
    glCompileShader(vid);
    check_compile_error(vid);

    const char* fs{ &fragment_source[0] };
    glShaderSource(fid, 1, &fs, nullptr);
    glCompileShader(fid);
    check_compile_error(fid);

    glAttachShader(program, vid);
    glAttachShader(program, fid);
    glLinkProgram(program);
    check_link_error(program);

    glDeleteShader(vid);
    glDeleteShader(fid);
  }

  void shader::destroy(std::uint32_t program)
  {
    glDeleteProgram(program);
  }

  void shader::check_compile_error(std::uint32_t shader)
  {
    std::int32_t status{};
    std::int32_t length{};
    std::string log{};

    glGetProgramiv(shader, GL_COMPILE_STATUS, &status);
    glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);
    
    log.resize(length);

    if (!status)
    {
      glGetProgramInfoLog(shader, length, nullptr, &log[0]);

      if (length)
      {
        std::printf("%s\n", &log[0]);
      }
    }
  }

  void shader::check_link_error(std::uint32_t program)
  {
    std::int32_t status{};
    std::int32_t length{};
    std::string log{};

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

    log.resize(length);

    if (!status)
    {
      glGetProgramInfoLog(program, length, nullptr, &log[0]);

      if (length)
      {
        std::printf("%s\n", &log[0]);
      }
    }
  }
}