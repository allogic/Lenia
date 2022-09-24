#ifndef WE_SHADER_H
#define WE_SHADER_H

#include <cstdint>
#include <string>

namespace we
{
  class shader
  {
  public:
    inline static const std::string s_rect_vertex_source
    {
      R"glsl(#version 460 core
      
      layout (location = 0) in vec4 i_position;
      layout (location = 1) in vec4 i_uv;
      
      layout (location = 0) out Forward
      {
        vec4 uv;
      } o_fwd;
      
      void main()
      {
        o_fwd.uv = i_uv;
        gl_Position = i_position;
      }
      )glsl"
    };
    inline static const std::string s_rect_fragment_source
    {
      R"glsl(#version 460 core
      
      layout (location = 0) in Forward
      {
        vec4 uv;
      } i_fwd;
      
      layout (location = 0) out vec4 o_color;
      
      layout (location = 0) uniform sampler2D u_texture;
      
      void main()
      {
        o_color = texture(u_texture, i_fwd.uv.xy);
      }
      )glsl"
    };

  public:
    shader() = delete;

  public:
    static void create(std::uint32_t& program, const std::string& vertex_source, const std::string& fragment_source);

    static void destroy(std::uint32_t program);

  private:
    static void check_compile_error(std::uint32_t shader);
    static void check_link_error(std::uint32_t program);
  };
}

#endif