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

///////////////////////////////////////////////////////////
// Locals
///////////////////////////////////////////////////////////

static const std::uint32_t s_window_width{ 1920 };
static const std::uint32_t s_window_height{ 1080 };

static std::float_t s_time{};
static std::float_t s_time_prev{};
static std::float_t s_time_delta{};

static std::uint32_t s_time_update_fps{ 120 };
static std::float_t s_time_update_prev{};

static const std::uint32_t s_system_count_x{ 10 };
static const std::uint32_t s_system_count_y{ 5 };
static const std::uint32_t s_system_width{ 256 };
static const std::uint32_t s_system_height{ 256 };

static std::vector<we::system*> s_systems{};

///////////////////////////////////////////////////////////
// Math stuff
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// UI
///////////////////////////////////////////////////////////

void ui_simulation()
{
  ImGui::Begin("Simulation Controls");

  ImGui::SliderInt("Fps", reinterpret_cast<std::int32_t*>(&s_time_update_fps), 0, 1000);
  if (ImGui::Button("Randomize"))
  {
    for (std::uint32_t i{}; i < s_systems.size(); i++)
    {
      s_systems[i]->randomize();
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

            // Update systems
            for (std::uint32_t i{}; i < s_systems.size(); i++)
            {
              s_systems[i]->update();
            }

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