#include <vao.h>

namespace we
{
  void vao::create(std::uint32_t& vao, std::uint32_t vertex_count, const vertex* vertices, std::uint32_t element_count, const std::uint32_t* elements)
  {
    std::uint32_t vbo{};
    std::uint32_t ebo{};

    glGenVertexArrays(1, &vao);

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vertex), vertices, GL_STATIC_READ | GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(0));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(std::float_t) * 4));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_count * sizeof(std::uint32_t), elements, GL_STATIC_READ | GL_STATIC_DRAW);

    glBindVertexArray(0);
  }
}