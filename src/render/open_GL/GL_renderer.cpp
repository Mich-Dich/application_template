#include "util/pch.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
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
        
        glfwMakeContextCurrent(m_window->get_window());
        
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

        // #ifdef DEBUG
        //     glGenQueries(1, &m_total_render_time);
        // #endif
    }
    
    GL_renderer::~GL_renderer() {
        
        resource_free();
        serialize(serializer::option::save_to_file);
    }
    



    void GL_renderer::draw_frame(float delta_time) {
            
        // execute_pending_commands();
        
        // if (m_render_world_viewport && m_viewport_size.x > 0 && m_viewport_size.y > 0) {
    
        // #ifdef DEBUG
        //     m_general_performance_metrik.next_iteration();
        //     glBeginQuery(GL_TIME_ELAPSED, m_total_render_time);
        // #endif

        //     // glUseProgram(m_shader_program);
        //     // glBindImageTexture(0, m_output_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        //     // // Dispatch compute shader
        //     // glm::uvec2 work_groups = glm::ceil(glm::vec2(m_viewport_size.x, m_viewport_size.y) / glm::vec2(8, 8));
        //     // glDispatchCompute(work_groups.x, work_groups.y, 1);
        //     // glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

        // #ifdef DEBUG        // set performance stuff
        //     m_general_performance_metrik.mesh_instances = component_group.size();
        //     glEndQuery(GL_TIME_ELAPSED);
        //     GLint available = 0;
        //     while (!available)
        //         glGetQueryObjectiv(m_total_render_time, GL_QUERY_RESULT_AVAILABLE, &available);

        //     GLuint64 elapsed_time;
        //     glGetQueryObjectui64v(m_total_render_time, GL_QUERY_RESULT, &elapsed_time);
        //     m_general_performance_metrik.renderer_draw_time[m_general_performance_metrik.current_index] = elapsed_time / 1e6f;
        //     m_general_performance_metrik.vertices = m_global_vertex_offset;
        // #endif
        // }

        if (m_imgui_initalized) {

            // ------ start new ImGui frame ------
		    ImGui::SetCurrentContext(application::get().get_imgui_config().get_context());
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            application::get().get_dashboard().draw(delta_time);
            
            ImGui::EndFrame();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
            // update other platform windows
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
            glfwSwapBuffers(m_window->get_window());
        }
    }

    // ================================================ imgui ================================================

    void GL_renderer::imgui_init() {

        IMGUI_CHECKVERSION();
        // ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
		// Viewport enable flags (require both ImGuiBackendFlags_PlatformHasViewports + ImGuiBackendFlags_RendererHasViewports set by the respective backends)
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows	
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

        ImGui_ImplGlfw_InitForOpenGL(m_window->get_window(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui::StyleColorsDark();
    }


    void GL_renderer::imgui_shutdown() {

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }


    void GL_renderer::imgui_create_fonts()      { ImGui_ImplOpenGL3_CreateFontsTexture(); }


    void GL_renderer::imgui_destroy_fonts()     { ImGui_ImplOpenGL3_DestroyFontsTexture(); }

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
        for (auto& command : m_shader_command_queue) {
            command();
        }
        m_shader_command_queue.clear();
    }

}
