#include <vector>
#include <random>
#include <fstream>

#include <texture.h>

#include <glad/glad.h>

namespace we
{
  void texture::create_fill(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, std::float_t value)
  {
    std::vector<std::float_t> values{};
    std::uint32_t size{ width * height * 4 };

    values.resize(size);

    for (std::uint32_t i{}; i < width; i++)
    {
      for (std::uint32_t j{}; j < height; j++)
      {
        std::uint32_t idx{ (i + j * width) * 4 };

        values[idx + 0] = value;
        values[idx + 1] = value;
        values[idx + 2] = value;
        values[idx + 3] = 1.0f;
      }
    }

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, &values[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void texture::create_random_r(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, std::float_t min, std::float_t max)
  {
    std::random_device random{};
    std::mt19937 generator{ random() };
    std::uniform_real_distribution<std::float_t> dist{ min, max };
    std::vector<std::float_t> values{};
    std::uint32_t size{ width * height * 4 };

    values.resize(size);

    for (std::uint32_t i{}; i < width; i++)
    {
      for (std::uint32_t j{}; j < height; j++)
      {
        std::uint32_t idx{ (i + j * width) * 4 };
        std::float_t v{ dist(generator) };

        values[idx + 0] = v;
        values[idx + 1] = v;
        values[idx + 2] = v;
        values[idx + 3] = 1.0f;
      }
    }

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, &values[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void texture::create_random_rgb(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, std::float_t min, std::float_t max)
  {
    std::random_device random{};
    std::mt19937 generator{ random() };
    std::uniform_real_distribution<std::float_t> dist{ min, max };
    std::vector<std::float_t> values{};
    std::uint32_t size{ width * height * 4 };

    values.resize(size);

    for (std::uint32_t i{}; i < width; i++)
    {
      for (std::uint32_t j{}; j < height; j++)
      {
        std::uint32_t idx{ (i + j * width) * 4 };

        values[idx + 0] = dist(generator);
        values[idx + 1] = dist(generator);
        values[idx + 2] = dist(generator);
        values[idx + 3] = 1.0f;
      }
    }

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, &values[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void texture::create_from_file(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, const std::string& file)
  {
    std::ifstream stream{ file, std::ios::binary };
    std::vector<uint8_t> values{};

    struct tga_header
    {
      uint8_t id_length;
      uint8_t color_type;
      uint8_t image_type;
      uint8_t reserved[5];
      uint16_t x_origin;
      uint16_t y_origin;
      uint16_t width;
      uint16_t height;
      uint8_t depth;
      uint8_t attributes;
    };

    tga_header header{};
    std::uint32_t size{};

    if (stream.is_open())
    {
      stream.read((char*)&header, sizeof(header));

      size = (header.width * header.height * header.depth) / 8;

      values.resize(size);

      stream.seekg(header.id_length, std::ios::cur);
      stream.read((char*)&values[0], size);

      stream.close();
    }

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, &values[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void texture::create_from_values(std::uint32_t& texture, std::uint32_t width, std::uint32_t height, const std::vector<std::float_t>& values)
  {
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, &values[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void texture::destroy(std::uint32_t texture)
  {
    glDeleteTextures(1, &texture);
  }
}