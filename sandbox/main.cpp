#include <cstdio>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <random>
#include <format>
#include <cmath>

#include <glad/glad.h>

#include <glfw/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <system.h>

#define IMAGE_DIR "C:\\Users\\Michael\\Downloads\\wave-engine\\images\\"
#define SHADER_DIR "C:\\Users\\Michael\\Downloads\\wave-engine\\shaders\\"

///////////////////////////////////////////////////////////
// Locals
///////////////////////////////////////////////////////////

static const std::uint32_t s_window_width{ 1000 };
static const std::uint32_t s_window_height{ 1000 };

static std::float_t s_time{};
static std::float_t s_time_prev{};
static std::float_t s_time_delta{};

static std::uint32_t s_time_update_fps{ 250 };
static std::float_t s_time_update_prev{};

static const std::uint32_t s_system_count_x{ 1 };
static const std::uint32_t s_system_count_y{ 1 };
static const std::uint32_t s_system_width{ 1000 };
static const std::uint32_t s_system_height{ 1000 };

static std::vector<we::system*> s_systems{};

///////////////////////////////////////////////////////////
// Math stuff
///////////////////////////////////////////////////////////

std::float_t bump(std::float_t x, std::float_t h, std::float_t o, std::float_t s, std::float_t n)
{
  return (h / (1.0f + std::powf(std::fabsf((x - o) / s), n))) - 1.0f;
}

///////////////////////////////////////////////////////////
// UI
///////////////////////////////////////////////////////////

void ui_simulation()
{
  ImGui::Begin("Simulation Controls");

  ImGui::SliderInt("Fps", reinterpret_cast<std::int32_t*>(&s_time_update_fps), 0, 1000);

  ImGui::End();
}

void ui_kernel_generation()
{
  static uint32_t kernel_texture;
  static bool is_dirty = true;
  static char shader_name[64] = "conv_32_test.frag";
  static std::vector<float> kernel;
  static float growth[64];

  ImGui::Begin("Kernel Generator");

  static int32_t kernel_size = 31;
  static float kernel_offset = 12.0f;
  static float kernel_distance = 300.0f;
  static float kernel_sharpness = 6.0f;

  ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
  if (ImGui::DragInt("##Kernel Size", &kernel_size, 1.0f, 1, 100, "Size %d")) is_dirty = true;
  if (ImGui::DragFloat("##Kernel Offset", &kernel_offset, 1.0f, 0.0f, 0.0f, "Offset %.3f")) is_dirty = true;
  if (ImGui::DragFloat("##Kernel Distance", &kernel_distance, 1.0f, 0.0f, 0.0f, "Distance %.3f")) is_dirty = true;
  if (ImGui::DragFloat("##Kernel Sharpness", &kernel_sharpness, 1.0f, 0.0f, 0.0f, "Sharpness %.3f")) is_dirty = true;
  ImGui::PopItemWidth();

  if (is_dirty)
  {
    is_dirty = false;

    glDeleteTextures(1, &kernel_texture);
    glGenTextures(1, &kernel_texture);
    glBindTexture(GL_TEXTURE_2D, kernel_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kernel_size, kernel_size, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    kernel.resize(kernel_size * kernel_size * 4);

    for (int32_t i = 0; i < kernel_size; i++)
    {
      for (int32_t j = 0; j < kernel_size; j++)
      {
        int32_t idx = (i + j * kernel_size) * 4;
        float h = static_cast<float>(kernel_size) / 2.0f;
        float x = static_cast<float>(i) - (h - 0.5f);
        float y = static_cast<float>(j) - (h - 0.5f);
        float l = std::sqrtf(x * x + y * y);
        float m = l + kernel_offset;
        float s = std::sinf((m * m) / kernel_distance);
        float v = std::powf(s, kernel_sharpness);
        if (l > (static_cast<float>(kernel_size) / 2.0f))
        {
          v = 0.0f;
        }
        kernel[idx + 0] = v;
        kernel[idx + 1] = v;
        kernel[idx + 2] = v;
        kernel[idx + 3] = 1.0f;
      }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kernel_size, kernel_size, 0, GL_RGBA, GL_FLOAT, &kernel[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  ImGui::Image(reinterpret_cast<void*>(static_cast<uint64_t>(kernel_texture)), { 256.0f, 256.0f });

  ImGui::Separator();

  static float growth_height = 2.0f;
  static float growth_offset = 2.0f;
  static float growth_smoothness = 1.0f;
  static float growth_sharpness = 2.0f;

  ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
  ImGui::DragFloat("##Growth Height", &growth_height, 0.1f, 0.0f, 5.0f, "Height %.3f");
  ImGui::DragFloat("##Growth Offset", &growth_offset, 0.1f, 0.0f, static_cast<float>(kernel_size), "Offset %.3f");
  ImGui::DragFloat("##Growth Smoothness", &growth_smoothness, 0.1f, 0.0f, 10.0f, "Smoothness %.3f");
  ImGui::DragFloat("##Growth Sharpness", &growth_sharpness, 0.1f, 0.0f, 10.0f, "Sharpness %.3f");
  ImGui::PopItemWidth();

  for (int32_t i = -10; i < 54; i++)
  {
    growth[i + 10] = bump(static_cast<float>(i), growth_height, growth_offset, growth_smoothness, growth_sharpness);
  }
  ImGui::PlotLines("", &growth[0], 64, 0, "Growth", -2.0f, 2.0f, { 256.0f, 100.0f });

  ImGui::InputText("Name", shader_name, 64);
  if (ImGui::Button("Export"))
  {
    std::ofstream stream{ std::string{ SHADER_DIR } + shader_name };

    if (stream.is_open())
    {
      stream << "#version 460 core\n\n";
      stream << "layout (location = 0) in Forward\n{\n  vec4 uv;\n} i_fwd;\n\n";
      stream << "layout (location = 0) out vec4 o_color;\n\n";
      stream << "layout (location = 0) uniform sampler2D u_texture;\n";
      stream << "layout (location = 1) uniform float u_delta_time;\n";
      stream << "layout (location = 2) uniform vec2 u_texture_size;\n\n";

      stream << "const float c_kernel[" << kernel_size << "][" << kernel_size << "] =\n{\n";
      for (int32_t i = 0; i < kernel_size; i++)
      {
        stream << "  { ";
        for (int32_t j = 0; j < kernel_size; j++)
        {
          int32_t idx = (i + j * kernel_size) * 4;
          stream << std::format("{:.7f}", kernel[idx]) << ", ";
        }
        stream << "},\n";
      }
      stream << "};\n\n";

      stream << "float growth(float x)\n{\n";
      stream << "  return (" << std::format("{:.7f}", growth_height) << " / (1.0f + pow(abs((x - " << std::format("{:.7f}", growth_offset) << ") / " << std::format("{:.7f}", growth_smoothness) << "), " << std::format("{:.7f}", growth_sharpness) << "))) - 1.0f;\n";
      stream << "}\n\n";

      stream << "void main()\n{\n";
      stream << "  vec4 sum = vec4(0.0);\n\n";
      stream << "  float fx = 1.0 / u_texture_size.x;\n";
      stream << "  float fy = 1.0 / u_texture_size.y;\n\n";
      stream << "  for (int i = 0; i < " << kernel_size << "; i++)\n  {\n";
      stream << "    for (int j = 0; j < " << kernel_size << "; j++)\n    {\n";
      stream << "      float u = fx * (float(i) - " << (static_cast<float>(kernel_size) / 2) << ");\n";
      stream << "      float v = fy * (float(j) - " << (static_cast<float>(kernel_size) / 2) << ");\n";
      stream << "      sum += c_kernel[i][j] * texture(u_texture, i_fwd.uv.xy + vec2(u, v));\n";
      stream << "    }\n";
      stream << "  }\n\n";
      stream << "  float r = texture(u_texture, i_fwd.uv.xy).r;\n";
      stream << "  float g = growth(sum.x) * u_delta_time;\n";
      stream << "  float c = clamp(r + g, 0.0, 1.0);\n\n";
      stream << "  o_color = vec4(c, c, c, 1.0);\n";
      stream << "}";

      stream.close();
    }
  }

  ImGui::End();
}

void ui_systems()
{
  ImGui::Begin("System Controls");

  for (std::uint32_t i{}; i < s_systems.size(); i++)
  {
    s_systems[i]->ui();
  }

  ImGui::End();
}

///////////////////////////////////////////////////////////
// Entry point
///////////////////////////////////////////////////////////

std::int32_t main()
{
  // Initialize glfw
  if (glfwInit())
  {
    // Setup glfw
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    GLFWwindow* window{ glfwCreateWindow(s_window_width, s_window_height, "Sandbox", nullptr, nullptr) };
    if (window)
    {
      // Make context current
      glfwMakeContextCurrent(window);

      // Load gl
      if (gladLoadGL())
      {
        // Set swap interval
        glfwSwapInterval(0);

        // Create imgui context
        IMGUI_CHECKVERSION();
        ImGuiContext* imgui_context{ ImGui::CreateContext() };

        // Setup imgui
        ImGuiIO& io{ ImGui::GetIO() };
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGuiStyle& style{ ImGui::GetStyle() };
        style.WindowRounding = 0.0f;
        style.FrameBorderSize = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;

        // Init imgui
        bool imgui_glfw_init{ ImGui_ImplGlfw_InitForOpenGL(window, true) };
        bool imgui_ogl_init{ ImGui_ImplOpenGL3_Init("#version 460 core") };
        if (imgui_context && imgui_glfw_init && imgui_ogl_init)
        {
          // Create systems
          s_systems.resize(s_system_count_x * s_system_count_y);
          for (std::uint32_t i{}; i < s_systems.size(); i++)
          {
            s_systems[i] = new we::system{ s_system_width, s_system_height, 10, 10 };
          }

          while (!glfwWindowShouldClose(window))
          {
            // Compute time
            s_time = static_cast<std::float_t>(glfwGetTime());
            s_time_delta = s_time - s_time_prev;
            s_time_prev = s_time;

            // Capped loop
            if ((s_time - s_time_update_prev) >= (1.0f / s_time_update_fps))
            {
              s_time_update_prev = s_time;

              // Swap system buffers
              for (std::uint32_t i{}; i < s_systems.size(); i++)
              {
                s_systems[i]->swap();
              }
            }

            // Set viewport to window size
            glViewport(0, 0, s_window_width, s_window_height);

            // Clear screen
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Display systems
            std::float_t system_scale_x{ static_cast<std::float_t>(s_window_width) / (s_system_count_x * s_system_width) };
            std::float_t system_scale_y{ static_cast<std::float_t>(s_window_height) / (s_system_count_y * s_system_height) };

            std::float_t system_width{ static_cast<std::float_t>(s_system_width) * system_scale_x };
            std::float_t system_height{ static_cast<std::float_t>(s_system_height) * system_scale_y };

            for (std::uint32_t x{}; x < s_system_count_x; x++)
            {
              for (std::uint32_t y{}; y < s_system_count_y; y++)
              {
                std::float_t system_pos_x{ static_cast<std::float_t>(x) * system_width };
                std::float_t system_pos_y{ static_cast<std::float_t>(y) * system_height };
                s_systems[x + y * s_system_count_x]->draw(system_pos_x, system_pos_y, system_scale_x, system_scale_y);
              }
            }

            // Render imgui
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

            // Draw controls
            ui_simulation();
            ui_kernel_generation();
            ui_systems();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();

            // Make context current
            glfwMakeContextCurrent(window);

            // Swap buffers
            glfwSwapBuffers(window);

            // Poll events
            glfwPollEvents();
          }

          // Terminate imgui
          ImGui_ImplOpenGL3_Shutdown();
          ImGui_ImplGlfw_Shutdown();
          ImGui::DestroyContext();
        }
        else
        {
          std::printf("Failed initializing imgui\n");
        }
      }
      else
      {
        std::printf("Failed loading GL\n");
      }

      // Terminate glfw
      glfwDestroyWindow(window);
      glfwTerminate();
    }
    else
    {
    std::printf("Failed creating window\n");
    }
  }
  else
  {
    std::printf("Failed initializing GLFW\n");
  }

  return 0;
}