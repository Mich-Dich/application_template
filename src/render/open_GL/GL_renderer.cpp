#include "util/pch.h"

#if defined(PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GL/glew.h>

#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
    #include <GLFW/glfw3.h>
    #include <imgui_impl_glfw.h>
#else
    #include <SDL3/SDL.h>
    #include <imgui_impl_sdl3.h>
#endif

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>

#include "util/util.h"
#include "application.h"
#include "platform/window.h"
#include "render/image.h"
#include "config/imgui_config.h"
#include "dashboard/dashboard.h"

#include "GL_renderer.h"


namespace AT::render::open_GL {


    GL_renderer::GL_renderer(ref<window> window)
        : renderer(window) {

        PROFILE_APPLICATION_FUNCTION();

        #if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
            glfwMakeContextCurrent(m_window->get_window());
        #else
            SDL_GL_MakeCurrent(m_window->get_window(), m_window->get_gl_context());
        #endif

        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        ASSERT(err == GLEW_OK, "", "Failed to initialize GLEW: " << glewGetErrorString(err))
        ASSERT(GLEW_VERSION_4_6, "", "OpenGL 4.6 not supported!")

        LOG(Trace, "OpenGL Version: " << glGetString(GL_VERSION));
        LOG(Trace, "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION));
        LOG(Trace, "Vendor: " << glGetString(GL_VENDOR));
        LOG(Trace, "Renderer: " << glGetString(GL_RENDERER));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        serialize(serializer::option::load_from_file);

        m_window_size = glm::ivec2((f32)m_window->get_width(), (f32)m_window->get_height());

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        LOG_INIT
    }

    GL_renderer::~GL_renderer() {

        resource_free();
        serialize(serializer::option::save_to_file);
    }



    void GL_renderer::draw_frame(float delta_time) {

        PROFILE_APPLICATION_FUNCTION();

        if (m_state != system_state::active)
            return;

        // execute_pending_commands();              // DISABLED: dont need custom shaders yet

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (m_imgui_initalized) {

            {
                PROFILE_RENDER_SCOPE("Start new ImGui Frame");
                ImGui::SetCurrentContext(application::get().get_imgui_config_ref()->get_context_imgui());
                ImGui_ImplOpenGL3_NewFrame();
                #if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
                    ImGui_ImplGlfw_NewFrame();
                #else
                    ImGui_ImplSDL3_NewFrame();
                #endif
                ImGui::NewFrame();
            }

            {
                PROFILE_RENDER_SCOPE("dashboard draw");
                application::get().get_dashboard()->draw(delta_time);
            }

            {
                PROFILE_RENDER_SCOPE("End Frame");
                ImGui::EndFrame();
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            {
                PROFILE_RENDER_SCOPE("Render external windows");

                #if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
                    GLFWwindow* backup_current_context = glfwGetCurrentContext();
                #else
                    SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
                    SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
                #endif

                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();

                #if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
                    glfwMakeContextCurrent(backup_current_context);
                    glfwSwapBuffers(m_window->get_window());
                #else
                    SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
                    SDL_GL_SwapWindow(m_window->get_window());
                #endif
            }
        }
    }


    void GL_renderer::draw_startup_UI(float delta_time) {

        PROFILE_APPLICATION_FUNCTION();

        // execute_pending_commands();              // DISABLED: dont need custom shaders yet

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (m_imgui_initalized) {

            // ------ start new ImGui frame ------
		    ImGui::SetCurrentContext(application::get().get_imgui_config_ref()->get_context_imgui());
            ImGui_ImplOpenGL3_NewFrame();
            #if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
                ImGui_ImplGlfw_NewFrame();
            #else
                ImGui_ImplSDL3_NewFrame();
            #endif
            ImGui::NewFrame();

            application::get()->draw_init_UI(delta_time);

            ImGui::EndFrame();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // update other platform windows
            #if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
            #else
                SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
                SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            #endif

            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();

            #if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
                glfwMakeContextCurrent(backup_current_context);
                glfwSwapBuffers(m_window->get_window());
            #else
                SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
                SDL_GL_SwapWindow(m_window->get_window());
            #endif
        }
    }

    // ================================================ imgui ================================================

    void GL_renderer::imgui_init() {

        IMGUI_CHECKVERSION();
        ImGuiIO& io = ImGui::GetIO();

        std::filesystem::path ini_path = config::get_filepath_from_configtype_ini(util::get_executable_path(), config::file::imgui);
        io.IniFilename = ImStrdup(ini_path.string().c_str());

		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
		// Viewport enable flags (require both ImGuiBackendFlags_PlatformHasViewports + ImGuiBackendFlags_RendererHasViewports set by the respective backends)
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    	// io.IniFilename = (util::get_executable_path().parent_path() / "config" / "imgui.ini").generic_string().c_str();

        // util::get_executable_path();

        #if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
            ImGui_ImplGlfw_InitForOpenGL(m_window->get_window(), true);
        #else
            ImGui_ImplSDL3_InitForOpenGL(m_window->get_window(), m_window->get_gl_context());
        #endif
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui::StyleColorsDark();
        m_imgui_initalized = true;
    }


    void GL_renderer::imgui_shutdown() {

        ImGui_ImplOpenGL3_Shutdown();
        #if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
            ImGui_ImplGlfw_Shutdown();
        #else
            ImGui_ImplSDL3_Shutdown();
        #endif
    }


    void GL_renderer::imgui_create_fonts()      { /* Texture handling was made automatic, no need to call this anymore: ImGui_ImplOpenGL3_CreateFontsTexture(); */ }


    void GL_renderer::imgui_destroy_fonts()     { /* Texture handling was made automatic, no need to call this anymore: ImGui_ImplOpenGL3_DestroyFontsTexture(); */ }

    // ================================================ utility ================================================

    void GL_renderer::serialize(serializer::option option) {

    }


    void GL_renderer::set_window_size(const u32 width, const u32 height) {                  // nothing for OpenGL

        m_window_size = glm::ivec2(width, height);
    }


    void GL_renderer::resource_free() {

        // #ifdef DEBUG
        // if (m_total_render_time) {
        //     glDeleteQueries(1, &m_total_render_time);
        //     m_total_render_time = 0;
        // }
        // #endif
    }

    // ================================================ shader ================================================

    void GL_renderer::execute_pending_commands() {

        for (auto& command : m_shader_command_queue)
            command();

        m_shader_command_queue.clear();
    }

}
