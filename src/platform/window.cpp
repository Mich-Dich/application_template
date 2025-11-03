
#include "util/pch.h"

#include <stb_image.h>
#include <GL/glew.h>
#include <imgui.h>

#if defined(PLATFORm_windowS)
    #include <Windows.h>
	#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
    	#define GLFW_EXPOSE_NATIVE_WIN32
	#endif
#endif

#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
	#include <GLFW/glfw3.h>
	#if defined(PLATFORm_windowS)
		#include <GLFW/glfw3native.h>
	#endif
#else
	#include <SDL3/SDL.h>
	#include <SDL3/SDL_opengl.h>
	#include <SDL3/SDL_mouse.h>
	#include <imgui_impl_sdl3.h>
#endif

#if defined(RENDER_API_OPENGL)
	#include <imgui_impl_opengl3.h>
#else
	#include <imgui_impl_vulkan.h>
#endif

#include "application.h"
#include "render/renderer.h"
#include "events/event.h"
#include "events/application_event.h"
#include "events/mouse_event.h"
#include "events/key_event.h"
#include "util/io/serializer_yaml.h"

#include "window.h"


namespace AT {


	static bool s_backend_initialized = false;

	#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
	
		static FORCEINLINE void GLFW_error_callback(int errorCode, const char* description) { LOG(Error, "[GLFW Error: " << errorCode << "]: " << description); }
		
		static GLFWimage load_icon(const std::string& filepath) {

			GLFWimage icon = {};
			int channels;
			icon.pixels = stbi_load(filepath.c_str(), &icon.width, &icon.height, &channels, 4);			// Load image data
			if (!icon.pixels)
				LOG(Error, "Failed to load window icon: " << filepath);
			
			return icon;
		}

	#else
		
		key_code sdlScancodeToInternalKeycode(const SDL_Scancode code) {
			switch (code) {
				case SDL_SCANCODE_A:                    return key_code::key_A;
				case SDL_SCANCODE_B:                    return key_code::key_B;
				case SDL_SCANCODE_C:                    return key_code::key_C;
				case SDL_SCANCODE_D:                    return key_code::key_D;
				case SDL_SCANCODE_E:                    return key_code::key_E;
				case SDL_SCANCODE_F:                    return key_code::key_F;
				case SDL_SCANCODE_G:                    return key_code::key_G;
				case SDL_SCANCODE_H:                    return key_code::key_H;
				case SDL_SCANCODE_I:                    return key_code::key_I;
				case SDL_SCANCODE_J:                    return key_code::key_J;
				case SDL_SCANCODE_K:                    return key_code::key_K;
				case SDL_SCANCODE_L:                    return key_code::key_L;
				case SDL_SCANCODE_M:                    return key_code::key_M;
				case SDL_SCANCODE_N:                    return key_code::key_N;
				case SDL_SCANCODE_O:                    return key_code::key_O;
				case SDL_SCANCODE_P:                    return key_code::key_P;
				case SDL_SCANCODE_Q:                    return key_code::key_Q;
				case SDL_SCANCODE_R:                    return key_code::key_R;
				case SDL_SCANCODE_S:                    return key_code::key_S;
				case SDL_SCANCODE_T:                    return key_code::key_T;
				case SDL_SCANCODE_U:                    return key_code::key_U;
				case SDL_SCANCODE_V:                    return key_code::key_V;
				case SDL_SCANCODE_W:                    return key_code::key_W;
				case SDL_SCANCODE_X:                    return key_code::key_X;
				case SDL_SCANCODE_Y:                    return key_code::key_Y;
				case SDL_SCANCODE_Z:                    return key_code::key_Z;
				case SDL_SCANCODE_1:                    return key_code::key_1;
				case SDL_SCANCODE_2:                    return key_code::key_2;
				case SDL_SCANCODE_3:                    return key_code::key_3;
				case SDL_SCANCODE_4:                    return key_code::key_4;
				case SDL_SCANCODE_5:                    return key_code::key_5;
				case SDL_SCANCODE_6:                    return key_code::key_6;
				case SDL_SCANCODE_7:                    return key_code::key_7;
				case SDL_SCANCODE_8:                    return key_code::key_8;
				case SDL_SCANCODE_9:                    return key_code::key_9;
				case SDL_SCANCODE_0:                    return key_code::key_0;
				case SDL_SCANCODE_RETURN:               return key_code::key_enter;
				case SDL_SCANCODE_ESCAPE:               return key_code::key_escape;
				case SDL_SCANCODE_BACKSPACE:            return key_code::key_backspace;
				case SDL_SCANCODE_TAB:                  return key_code::key_tab;
				case SDL_SCANCODE_SPACE:                return key_code::key_space;
				case SDL_SCANCODE_MINUS:                return key_code::key_minus;
				case SDL_SCANCODE_EQUALS:               return key_code::key_equal;
				case SDL_SCANCODE_LEFTBRACKET:          return key_code::key_left_bracket;
				case SDL_SCANCODE_RIGHTBRACKET:         return key_code::key_right_bracket;
				case SDL_SCANCODE_BACKSLASH:            return key_code::key_backslash;
				case SDL_SCANCODE_NONUSHASH:            return key_code::key_world_2; // Assuming this maps to world_2
				case SDL_SCANCODE_SEMICOLON:            return key_code::key_semicolon;
				case SDL_SCANCODE_APOSTROPHE:           return key_code::key_apostrophe;
				case SDL_SCANCODE_GRAVE:                return key_code::key_grave_accent;
				case SDL_SCANCODE_COMMA:                return key_code::key_comma;
				case SDL_SCANCODE_PERIOD:               return key_code::key_period;
				case SDL_SCANCODE_SLASH:                return key_code::key_slash;
				case SDL_SCANCODE_CAPSLOCK:             return key_code::key_caps_lock;
				case SDL_SCANCODE_F1:                   return key_code::key_F1;
				case SDL_SCANCODE_F2:                   return key_code::key_F2;
				case SDL_SCANCODE_F3:                   return key_code::key_F3;
				case SDL_SCANCODE_F4:                   return key_code::key_F4;
				case SDL_SCANCODE_F5:                   return key_code::key_F5;
				case SDL_SCANCODE_F6:                   return key_code::key_F6;
				case SDL_SCANCODE_F7:                   return key_code::key_F7;
				case SDL_SCANCODE_F8:                   return key_code::key_F8;
				case SDL_SCANCODE_F9:                   return key_code::key_F9;
				case SDL_SCANCODE_F10:                  return key_code::key_F10;
				case SDL_SCANCODE_F11:                  return key_code::key_F11;
				case SDL_SCANCODE_F12:                  return key_code::key_F12;
				case SDL_SCANCODE_PRINTSCREEN:          return key_code::key_print_screen;
				case SDL_SCANCODE_SCROLLLOCK:           return key_code::key_scroll_lock;
				case SDL_SCANCODE_PAUSE:                return key_code::key_pause;
				case SDL_SCANCODE_INSERT:               return key_code::key_insert;
				case SDL_SCANCODE_HOME:                 return key_code::key_home;
				case SDL_SCANCODE_PAGEUP:               return key_code::key_page_up;
				case SDL_SCANCODE_DELETE:               return key_code::key_delete;
				case SDL_SCANCODE_END:                  return key_code::key_end;
				case SDL_SCANCODE_PAGEDOWN:             return key_code::key_page_down;
				case SDL_SCANCODE_RIGHT:                return key_code::key_right;
				case SDL_SCANCODE_LEFT:                 return key_code::key_left;
				case SDL_SCANCODE_DOWN:                 return key_code::key_down;
				case SDL_SCANCODE_UP:                   return key_code::key_up;
				case SDL_SCANCODE_NUMLOCKCLEAR:         return key_code::key_num_lock;
				case SDL_SCANCODE_KP_DIVIDE:            return key_code::key_kp_divide;
				case SDL_SCANCODE_KP_MULTIPLY:          return key_code::key_kp_multiply;
				case SDL_SCANCODE_KP_MINUS:             return key_code::key_kp_subtrace;
				case SDL_SCANCODE_KP_PLUS:              return key_code::key_kp_add;
				case SDL_SCANCODE_KP_ENTER:             return key_code::key_kp_enter;
				case SDL_SCANCODE_KP_1:                 return key_code::key_kp_1;
				case SDL_SCANCODE_KP_2:                 return key_code::key_kp_2;
				case SDL_SCANCODE_KP_3:                 return key_code::key_kp_3;
				case SDL_SCANCODE_KP_4:                 return key_code::key_kp_4;
				case SDL_SCANCODE_KP_5:                 return key_code::key_kp_5;
				case SDL_SCANCODE_KP_6:                 return key_code::key_kp_6;
				case SDL_SCANCODE_KP_7:                 return key_code::key_kp_7;
				case SDL_SCANCODE_KP_8:                 return key_code::key_kp_8;
				case SDL_SCANCODE_KP_9:                 return key_code::key_kp_9;
				case SDL_SCANCODE_KP_0:                 return key_code::key_kp_0;
				case SDL_SCANCODE_KP_PERIOD:            return key_code::key_kp_decimal;
				case SDL_SCANCODE_NONUSBACKSLASH:       return key_code::key_world_1; // Assuming this maps to world_1
				case SDL_SCANCODE_APPLICATION:          return key_code::key_menu;
				case SDL_SCANCODE_KP_EQUALS:            return key_code::key_kp_equal;
				case SDL_SCANCODE_F13:                  return key_code::key_F13;
				case SDL_SCANCODE_F14:                  return key_code::key_F14;
				case SDL_SCANCODE_F15:                  return key_code::key_F15;
				case SDL_SCANCODE_F16:                  return key_code::key_F16;
				case SDL_SCANCODE_F17:                  return key_code::key_F17;
				case SDL_SCANCODE_F18:                  return key_code::key_F18;
				case SDL_SCANCODE_F19:                  return key_code::key_F19;
				case SDL_SCANCODE_F20:                  return key_code::key_F20;
				case SDL_SCANCODE_F21:                  return key_code::key_F21;
				case SDL_SCANCODE_F22:                  return key_code::key_F22;
				case SDL_SCANCODE_F23:                  return key_code::key_F23;
				case SDL_SCANCODE_F24:                  return key_code::key_F24;
				case SDL_SCANCODE_LCTRL:                return key_code::key_left_control;
				case SDL_SCANCODE_LSHIFT:               return key_code::key_left_shift;
				case SDL_SCANCODE_LALT:                 return key_code::key_left_alt;
				case SDL_SCANCODE_LGUI:                 return key_code::key_left_super;
				case SDL_SCANCODE_RCTRL:                return key_code::key_right_control;
				case SDL_SCANCODE_RSHIFT:               return key_code::key_right_shift;
				case SDL_SCANCODE_RALT:                 return key_code::key_right_alt;
				case SDL_SCANCODE_RGUI:                 return key_code::key_right_super;
				case SDL_SCANCODE_MENU:                 return key_code::key_menu;

				// For all other unmapped keys, return unknown
				default:                                return key_code::key_unknown;
			}
		}

		
		static FORCEINLINE void SDL_error_callback() { LOG(Error, "[SDL Error]: " << SDL_GetError()); }
		
		static SDL_Surface* load_icon(const std::string& filepath) {
			int width, height, channels;
			unsigned char* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
			if (!pixels) {
				LOG(Error, "Failed to load window icon: " << filepath);
				return nullptr;
			}
			
			SDL_Surface* icon = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, pixels, 4 * width);
			if (!icon) {
				LOG(Error, "Failed to create SDL surface from icon: " << filepath);
				stbi_image_free(pixels);
				return nullptr;
			}
			return icon;
		}

	#endif
	

	// ================================================================================== setup ==================================================================================
	
	window::window(window_attrib attributes) :
		m_data(attributes) {
	
        PROFILE_APPLICATION_FUNCTION();
		
		#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
		
			if (!s_backend_initialized) {
		
				glfwSetErrorCallback(GLFW_error_callback);
				ASSERT(glfwInit(), "GLFW initialized", "Could not initialize GLFW");
				s_backend_initialized = true;
			}
		
			#if defined(RENDER_API_OPENGL)
				glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
				glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
				#if defined(PLATFORM_APPLE)
					glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
				#endif
				glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
			#elif defined(RENDER_API_VULKAN)
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			#endif
		
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			glfwWindowHint(GLFW_MAXIMIZED, (m_data.size_state == window_size_state::fullscreen || m_data.size_state == window_size_state::fullscreen_windowed) ? GLFW_TRUE : GLFW_FALSE);
		
			int max_possible_height = 0;
			int max_possible_width = 0;
			int monitor_count;
			auto monitors = glfwGetMonitors(&monitor_count);
		
			for (int x = 0; x < monitor_count; x++) {
		
				int xpos, ypos, width, height;
				glfwGetMonitorWorkarea(monitors[x], &xpos, &ypos, &width, &height);
				max_possible_height = math::max(max_possible_height, height);
				max_possible_width = math::max(max_possible_width, width);
		
				LOG(Trace, "Monitor: " << x << " data: " << xpos << " / " << ypos << " / " << width << " / " << height);
			}

			// load data from [app_setting.yml]
			std::filesystem::path logo_path{};
			AT::serializer::yaml(config::get_filepath_from_configtype(util::get_executable_path(), config::file::app_settings), "general_settings", AT::serializer::option::load_from_file)
				.entry("display_name", m_data.title)
				.entry(KEY_VALUE(logo_path));
		
			// ensure window is never bigger than possible OR smaller then logical
			m_data.height = math::clamp((int)m_data.height, 200, max_possible_height);
			m_data.width = math::clamp((int)m_data.width, 300, max_possible_width);
			LOG(Trace, "Creating window [" << m_data.title << " width: " << m_data.width << "  height: " << m_data.height << "]");
			m_window = glfwCreateWindow(static_cast<int>(m_data.width), static_cast<int>(m_data.height), m_data.title.c_str(), nullptr, nullptr);
		
			if (!logo_path.empty()) {

				const auto icon_full_path = util::get_executable_path() / logo_path;
				if (std::filesystem::exists(icon_full_path)) {

					GLFWimage icon = load_icon(icon_full_path.string());
					if (icon.pixels) {
						glfwSetWindowIcon(m_window, 1, &icon);
						stbi_image_free(icon.pixels);
					}
				} else
					LOG(Error, "Icon file not found [" << icon_full_path << "]");
			}

			#if defined(RENDER_API_VULKAN)
				#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
					ASSERT(glfwVulkanSupported(), "", "GLFW does not support Vulkan");
				#else
				#endif
			#endif
		
			//glfwSetWindowPos(m_window, 100, 100);
			glfwSetWindowPos(m_window, m_data.pos_x, m_data.pos_y);
			LOG(Trace, "window pos [" << m_data.title << " X: " << m_data.pos_x << "  Y: " << m_data.pos_y << "]");
			
			glfwSetWindowUserPointer(m_window, &m_data);
			glfwGetCursorPos(m_window, &m_data.cursor_pos_x, &m_data.cursor_pos_y);
			set_vsync(m_data.vsync);
		
			GLFWmonitor* primary = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(primary);
			glfwSetWindowMonitor(m_window, NULL, m_data.pos_x, m_data.pos_y, m_data.width, m_data.height, mode->refreshRate);
		
			if (m_data.size_state == window_size_state::fullscreen
				|| m_data.size_state == window_size_state::fullscreen_windowed)
				glfwMaximizeWindow(m_window);
		
			bind_event_callbacks();
		
			f64 cursor_x, cursor_y;
			glfwGetCursorPos(m_window, &cursor_x, &cursor_y);
				
		#else
			
			if (!s_SDL_initialized)
			{
				ASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS), "", "SDL init failed [" << SDL_GetError() << "]");
							
				s_SDL_initialized = true;
				LOG(Info, "SDL initialized successfully");
			}
		
			Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
			
			if (m_data.size_state == window_size_state::fullscreen)
			{
				window_flags |= SDL_WINDOW_FULLSCREEN;
			}
			else if (m_data.size_state == window_size_state::fullscreen_windowed)
			{
				window_flags |= SDL_WINDOW_BORDERLESS;
			}
		
			// Set OpenGL attributes
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		
			int max_possible_height = 0;
			int max_possible_width = 0;
			int monitor_count = 0;
			SDL_GetDisplays(&monitor_count);

			for (int x = 0; x < monitor_count; x++)
			{
				SDL_Rect bounds;
				SDL_GetDisplayBounds(x, &bounds);
				max_possible_height = math::max(max_possible_height, bounds.h);
				max_possible_width = math::max(max_possible_width, bounds.w);
				LOG(Trace, "Monitor: " << x << " data: " << bounds.x << " / " << bounds.y << " / " << bounds.w << " / " << bounds.h);
			}
		
			// ensure window is never bigger than possible OR smaller then logical
			m_data.height = math::clamp((int)m_data.height, 200, max_possible_height);
			m_data.width = math::clamp((int)m_data.width, 300, max_possible_width);
			
			LOG(Trace, "Creating window [" << m_data.title << " width: " << m_data.width << "  height: " << m_data.height << "]");
			
			m_window = SDL_CreateWindow(m_data.title.c_str(), 
									static_cast<int>(m_data.width), 
									static_cast<int>(m_data.height), 
									window_flags);
		
			if (!m_window)
			{
				LOG(Error, "Failed to create SDL window: " << SDL_GetError());
				return;
			}
		
			// Set window icon
			const auto icon_full_path = ASSET_PATH / "images/logo.png";
			if (std::filesystem::exists(icon_full_path)) {
				SDL_Surface* icon = load_icon(icon_full_path.string());
				if (icon) {
					SDL_SetWindowIcon(m_window, icon);
					SDL_DestroySurface(icon);
				}
			} else
			{
				LOG(Error, "Icon file not found [" << icon_full_path << "]");
			}
		
			// Create OpenGL context
			m_gl_context = SDL_GL_CreateContext(m_window);
			if (!m_gl_context)
			{
				LOG(Error, "Failed to create OpenGL context: " << SDL_GetError());
			}
		
			SDL_SetWindowPosition(m_window, m_data.pos_x, m_data.pos_y);
			LOG(Trace, "window pos [" << m_data.title << " X: " << m_data.pos_x << "  Y: " << m_data.pos_y << "]");
			
			set_vsync(m_data.vsync);
			
			// Get initial cursor position
			float cursor_x, cursor_y;
			SDL_GetMouseState(&cursor_x, &cursor_y);
		#endif
		
		m_data.cursor_pos_x = cursor_x;
		m_data.cursor_pos_y = cursor_y;
		
		LOG_INIT
	}
	
	window::~window() {
		
		if (!s_backend_initialized || !m_window)
			return;

        PROFILE_APPLICATION_FUNCTION();
		
		const bool is_maximized = this->is_maximized();
		int titlebar_vertical_offset = is_maximized ? 12 : 6;
	
		int loc_pos_x, loc_pos_y;
		
		#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
		
			glfwGetWindowPos(m_window, &loc_pos_x, &loc_pos_y);
			m_data.pos_x = loc_pos_x + 4;								// window padding
			m_data.pos_y = loc_pos_y + 25 + titlebar_vertical_offset;	// [custom titlebar height offset] + [maximize offset]
		
			glfwDestroyWindow(m_window);
			glfwTerminate();

		#else
			
			SDL_GetWindowPosition(m_window, &loc_pos_x, &loc_pos_y);
			m_data.pos_x = loc_pos_x + 4;
			m_data.pos_y = loc_pos_y + 25 + titlebar_vertical_offset;
		
			if (m_gl_context)
				SDL_GL_DestroyContext(m_gl_context);
			
			SDL_DestroyWindow(m_window);
			SDL_Quit();

			#endif
			
		s_backend_initialized = false;
		LOG_SHUTDOWN
	}
	
	// ============================================================================== implementation ==============================================================================

	
	#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
		void window::bind_event_callbacks() {
	
			glfwSetWindowRefreshCallback(m_window, [](GLFWwindow* window) {
				
				window_attrib& data = *(window_attrib*)glfwGetWindowUserPointer(window);
				window_refresh_event event;
				data.event_callback(event);
			});
		
			glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		
				window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
		
				if (Data.size_state == window_size_state::windowed) {
		
					Data.width = static_cast<u32>(width);
					Data.height = static_cast<u32>(height);
				}
		
				window_resize_event event(static_cast<u32>(width), static_cast<u32>(height));
				Data.event_callback(event);
			});
		
			glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, int focused) {
		
				window_attrib& data = *(window_attrib*)glfwGetWindowUserPointer(window);
				window_focus_event event(focused == GLFW_TRUE);
				data.event_callback(event);
			});
		
			glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
				
				window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
				window_close_event event;
				Data.event_callback(event);
			});
		
			
			glfwSetTitlebarHitTestCallback(m_window, [](GLFWwindow* window, int x, int y, int* hit) {
				
				// *hit = application::get().get_is_titlebar_hovered();
			});


			glfwSetWindowPosCallback(m_window, [](GLFWwindow* window, int x, int y) {
		
			});
		
			glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow* window, int maximized) {
		
				window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
				Data.size_state = maximized? window_size_state::fullscreen : window_size_state::windowed;
					LOG(Trace, "Maximize window: " << maximized);
			});
		
			glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
		
				window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
				mouse_event event_x(key_code::mouse_scrolled_x, static_cast<f32>(xOffset));
				mouse_event event_y(key_code::mouse_scrolled_y, static_cast<f32>(yOffset));
				Data.event_callback(event_x);
				Data.event_callback(event_y);
			});
		
			glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
		
				window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
				mouse_event event_x(key_code::mouse_moved_x, static_cast<f32>(Data.cursor_pos_x - xPos));
				mouse_event event_y(key_code::mouse_moved_y, static_cast<f32>(Data.cursor_pos_y - yPos));
				Data.event_callback(event_x);
				Data.event_callback(event_y);
		
				Data.cursor_pos_x = xPos;
				Data.cursor_pos_y = yPos;
			});
		
			glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		
				window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
				key_event event(static_cast<key_code>(key), static_cast<key_state>(action));
				Data.event_callback(event);
			});
		
			glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
		
				window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
				key_event event(static_cast<key_code>(button), static_cast<key_state>(action));
				Data.event_callback(event);
			});
		
		}
	
	#else
		void window::process_sdl_event(const SDL_Event& event) {

			// Forward event to ImGui first
			ImGui_ImplSDL3_ProcessEvent(&event);
			
			// If ImGui wants to capture the event, don't process it further
			ImGuiIO& io = ImGui::GetIO();
			if ((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP ||
				event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_WHEEL) && io.WantCaptureMouse)
				return;

			// if ((event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) && io.WantCaptureKeyboard)
			// 	return;
			
			if (event.type == SDL_EVENT_TEXT_INPUT && io.WantTextInput)
				return;

			switch (event.type) {
				case SDL_EVENT_WINDOW_RESIZED: {
					window_resize_event resize_event(static_cast<u32>(event.window.data1), static_cast<u32>(event.window.data2));
					m_data.event_callback(resize_event);
					break;
				}
				
				case SDL_EVENT_WINDOW_EXPOSED: {
					window_refresh_event refresh_event;
					m_data.event_callback(refresh_event);
					break;
				}
				
				case SDL_EVENT_WINDOW_FOCUS_GAINED: {
					window_focus_event focus_event(true);
					m_data.event_callback(focus_event);
					break;
				}
				
				case SDL_EVENT_WINDOW_FOCUS_LOST: {
					window_focus_event focus_event(false);
					m_data.event_callback(focus_event);
					break;
				}
				
				case SDL_EVENT_QUIT: {
					window_close_event close_event;
					m_data.event_callback(close_event);
					m_should_close = true;
					break;
				}
				
				case SDL_EVENT_WINDOW_MAXIMIZED: {
					m_data.size_state = window_size_state::fullscreen_windowed;
					LOG(Trace, "Maximize window: true");
					break;
				}
				
				case SDL_EVENT_WINDOW_RESTORED: {
					m_data.size_state = window_size_state::windowed;
					LOG(Trace, "Maximize window: false");
					break;
				}
				
				case SDL_EVENT_MOUSE_WHEEL: {
					mouse_event event_x(key_code::mouse_scrolled_x, static_cast<f32>(event.wheel.x));
					mouse_event event_y(key_code::mouse_scrolled_y, static_cast<f32>(event.wheel.y));
					m_data.event_callback(event_x);
					m_data.event_callback(event_y);
					break;
				}
				
				case SDL_EVENT_MOUSE_MOTION: {
					float old_x = static_cast<float>(m_data.cursor_pos_x);
					float old_y = static_cast<float>(m_data.cursor_pos_y);
					mouse_event event_x(key_code::mouse_moved_x, old_x - event.motion.x);
					mouse_event event_y(key_code::mouse_moved_y, old_y - event.motion.y);
					m_data.event_callback(event_x);
					m_data.event_callback(event_y);
					
					m_data.cursor_pos_x = event.motion.x;
					m_data.cursor_pos_y = event.motion.y;
					break;
				}
				
				case SDL_EVENT_KEY_DOWN:
				case SDL_EVENT_KEY_UP: {
					key_state state = (event.type == SDL_EVENT_KEY_DOWN) ? key_state::press : key_state::release;
					key_event key_evt(sdlScancodeToInternalKeycode(event.key.scancode), state);
					m_data.event_callback(key_evt);
					break;
				}
				
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				case SDL_EVENT_MOUSE_BUTTON_UP: {
					key_state state = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? key_state::press : key_state::release;
					key_event mouse_evt(static_cast<key_code>(event.button.button), state);
					m_data.event_callback(mouse_evt);
					break;
				}
			}
		}
		
	#endif

	
	void window::poll_events() {
		
		#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
			
			glfwPollEvents();
		
		#else
		
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				process_sdl_event(event);
			}
		
		#endif

		// prosesss custom queue
		std::scoped_lock<std::mutex> lock(m_event_queue_mutex);
		while (m_event_queue.size() > 0) {
	
			auto& func = m_event_queue.front();
			func();
			m_event_queue.pop();
		}
	}
	
	#if defined(PLATFORM_WINDOWING_BACKEND_GLFW)
		
		void window::capture_cursor() { glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
		
		void window::release_cursor() { glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
		
		glm::ivec2 window::get_extend() { return { static_cast<u32>(m_data.width), static_cast<u32>(m_data.height) }; }
		
		bool window::should_close() { return glfwWindowShouldClose(m_window); }
		
		void window::get_framebuffer_size(int& width, int& height) { glfwGetFramebufferSize(m_window, &width, &height); }
		
		void window::show_window(bool show) { show ? glfwShowWindow(m_window) : glfwHideWindow(m_window); }
		
		bool window::is_maximized() { return static_cast<bool>(glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED)); }


		void window::minimize_window() {
			
			LOG(Trace, "minimizing window");
			glfwIconifyWindow(m_window);
			m_data.size_state = window_size_state::minimized;
			//application::set_render_state(system_state::inactive);
		}

		
		void window::restore_window() {
			
			LOG(Trace, "restoring window");
			glfwRestoreWindow(m_window); 
			//application::set_render_state(system_state::active);
		}

		
		void window::maximize_window() { 
			
			LOG(Trace, "maximizing window");
			glfwMaximizeWindow(m_window);
			m_data.size_state = window_size_state::fullscreen_windowed;
			//application::set_render_state(system_state::active);
		}
		

		void window::show_titlebar(bool show) {
			
			glfwWindowHint(GLFW_TITLEBAR, show ? GLFW_TRUE : GLFW_FALSE);
		}
		

		void window::get_monitor_size(int* width, int* height) {
		
			auto monitor = glfwGetWindowMonitor(m_window);
			if (!monitor)
				monitor = glfwGetPrimaryMonitor();
		
			auto video_mode = glfwGetVideoMode(monitor);
		
			*width = video_mode->width;
			*height = video_mode->height;
		}
		

		void window::get_mouse_position(glm::vec2& pos) {
		
			// Get window position and size
			int winX, winY, winWidth, winHeight;
			glfwGetWindowPos(m_window, &winX, &winY);
			glfwGetWindowSize(m_window, &winWidth, &winHeight);
		
			double screenX, screenY;
			glfwGetCursorPos(m_window, &screenX, &screenY);
		
			screenX += winX;
			screenY += winY;
		
			if (screenX >= winX && screenX <= winX + winWidth &&
				screenY >= winY && screenY <= winY + winHeight) {
		
				pos.x = static_cast<f32>(screenX - winX);
				pos.y = static_cast<f32>(screenY - winY);
			}
		}
		
	#else
	
		void window::capture_cursor() {

			float screen_x, screen_y;
			SDL_GetMouseState(&screen_x, &screen_y);
			m_cursor_position = {screen_x, screen_y};
			SDL_CaptureMouse(true);
			SDL_HideCursor();
		}

		
		void window::release_cursor() {
			SDL_ShowCursor();
			SDL_CaptureMouse(false);
			SDL_WarpMouseInWindow(m_window, m_cursor_position.x, m_cursor_position.y);
		}
		

		glm::ivec2 window::get_extend() { return { static_cast<u32>(m_data.width), static_cast<u32>(m_data.height) }; }
		
		bool window::should_close() { return m_should_close; }
		
		void window::get_framebuffer_size(int& width, int& height) { SDL_GetWindowSize(m_window, &width, &height); }
		
		void window::show_window(bool show) {

			if (show)
				SDL_ShowWindow(m_window);

			else
				SDL_HideWindow(m_window);
		}
		

		bool window::is_maximized() { return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MAXIMIZED) != 0; }


		void window::minimize_window() {

			LOG(Trace, "minimizing window");
			SDL_MinimizeWindow(m_window);
			m_data.size_state = window_size_state::minimized;
			//application::set_render_state(system_state::inactive);
		}
		

		void window::restore_window() {

			LOG(Trace, "restoring window");
			SDL_RestoreWindow(m_window);
			//application::set_render_state(system_state::active);
		}

		
		void window::maximize_window() {

			LOG(Trace, "maximizing window");
			SDL_MaximizeWindow(m_window);
			m_data.size_state = window_size_state::fullscreen_windowed;
			//application::set_render_state(system_state::active);
		}

		
		void window::show_titlebar(bool show) {
			// Note: SDL3 doesn't have direct titlebar control like GLFW
			// You might need to use SDL_SetWindowBordered or recreate the window
			SDL_SetWindowBordered(m_window, show);
		}
		

		void window::get_monitor_size(int* width, int* height) {

			int display_index = SDL_GetDisplayForWindow(m_window);
			if (display_index < 0)
				display_index = 0;
			
			SDL_Rect bounds;
			SDL_GetDisplayBounds(display_index, &bounds);
			*width = bounds.w;
			*height = bounds.h;
		}
		

		void window::get_mouse_position(glm::vec2& pos) {

			int winX, winY, winWidth, winHeight;
			SDL_GetWindowPosition(m_window, &winX, &winY);
			SDL_GetWindowSize(m_window, &winWidth, &winHeight);
		
			float screenX, screenY;
			SDL_GetMouseState(&screenX, &screenY);
		
			screenX += winX;
			screenY += winY;
		
			if (screenX >= winX && screenX <= winX + winWidth && screenY >= winY && screenY <= winY + winHeight) {
				pos.x = static_cast<f32>(screenX - winX);
				pos.y = static_cast<f32>(screenY - winY);
			}
		}
		
	#endif
	
#if defined(RENDER_API_VULKAN)
	void window::create_vulkan_surface(VkInstance_T* instance, VkSurfaceKHR_T** get_surface) {
	
		ASSERT(glfwCreateWindowSurface(instance, m_window, nullptr, get_surface) == VK_SUCCESS, "", "Failed to create a window surface");
	}
#endif

}	
