
#include "util/pch.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "util/system.h"
#include "util/io/serializer_yaml.h"
#include "util/ui/panel_collection.h"
#include "util/ui/window_images.embed"

#include "application.h"
#include "render/renderer.h"
//#include "render/image.h"

#include "imgui_config.h"


#define UI_ACTIVE_THEME						AT::UI::THEME::current_theme

#define LERP_GRAY_A(value, alpha)			{value, value, value, alpha }
//#define COLOR_INT_CONVERSION(color)		IM_COL32(255 * color.x, 255 * color.y, 255 * color.z, 255 * color.w)
#define LERP_MAIN_COLOR_DARK(value)			{main_color.x * value, main_color.y * value, main_color.z * value, 1.f }		// Set [w] to be [1.f] to disable accidental transparency
#define LERP_MAIN_COLOR_LIGHT(value)		{	(1.f - value) * 1.f + value * main_color.x, \
												(1.f - value) * 1.f + value * main_color.y, \
												(1.f - value) * 1.f + value * main_color.z, \
												1.f }																		// Set [w] to be [1.f] to disable accidental transparency

namespace AT::UI {


	GETTER_REF_FUNC_IMPL(ImVec4, main_color);
	GETTER_REF_FUNC_IMPL(ImVec4, main_titlebar_color);

	GETTER_REF_FUNC_IMPL(ImVec4, action_color_00_faded);
	GETTER_REF_FUNC_IMPL(ImVec4, action_color_00_weak);
	GETTER_REF_FUNC_IMPL(ImVec4, action_color_00_default);
	GETTER_REF_FUNC_IMPL(ImVec4, action_color_00_hover);
	GETTER_REF_FUNC_IMPL(ImVec4, action_color_00_active);
			  
	GETTER_REF_FUNC_IMPL2(ImVec4, default_gray, IMCOLOR_GRAY(30));
	GETTER_REF_FUNC_IMPL2(ImVec4, default_gray_1, IMCOLOR_GRAY(35));
			  
	GETTER_REF_FUNC_IMPL2(ImVec4, action_color_gray_default, LERP_GRAY(0.2f));
	GETTER_REF_FUNC_IMPL2(ImVec4, action_color_gray_hover, LERP_GRAY(0.27f));
	GETTER_REF_FUNC_IMPL2(ImVec4, action_color_gray_active, LERP_GRAY(0.35f));

	[[maybe_unused]] static ImVec4 vector_multi(const ImVec4& vec_0, const ImVec4& vec_1) {
		return ImVec4{ vec_0.x * vec_1.x, vec_0.y * vec_1.y, vec_0.z * vec_1.z, vec_0.w * vec_1.w };
	}

	void set_UI_theme_selection(theme_selection theme_selection) { g_UI_theme = theme_selection; }


	void imgui_config::load_fonts() {

		ImGui::SetCurrentContext(m_context_imgui);
		ImPlot::SetCurrentContext(m_context_implot);

		auto& io = ImGui::GetIO();
		application::get().get_renderer()->imgui_destroy_fonts();
		io.Fonts->Clear();			// Clear the font atlas before adding new fonts
		m_fonts.clear();

		std::filesystem::path base_path = AT::util::get_executable_path() / "assets" / "fonts";
		std::filesystem::path font_path = base_path / "Open_Sans" / "static";
		std::filesystem::path Inconsolata_path = base_path / "Inconsolata" / "static";

		io.FontAllowUserScaling = true;
		m_fonts[font_type::regular] =		io.Fonts->AddFontFromFileTTF((font_path/ "OpenSans-Regular.ttf").string().c_str(), g_font_size);
		m_fonts[font_type::bold] =			io.Fonts->AddFontFromFileTTF((font_path/ "OpenSans-Bold.ttf").string().c_str(), g_font_size);
		m_fonts[font_type::italic] =		io.Fonts->AddFontFromFileTTF((font_path/ "OpenSans-Italic.ttf").string().c_str(), g_font_size);

		m_fonts[font_type::regular_big] =	io.Fonts->AddFontFromFileTTF((font_path / "OpenSans-Regular.ttf").string().c_str(), g_big_font_size);
		m_fonts[font_type::bold_big] =		io.Fonts->AddFontFromFileTTF((font_path / "OpenSans-Bold.ttf").string().c_str(), g_big_font_size);
		m_fonts[font_type::italic_big] =	io.Fonts->AddFontFromFileTTF((font_path / "OpenSans-Italic.ttf").string().c_str(), g_big_font_size);

		m_fonts[font_type::header_0] =		io.Fonts->AddFontFromFileTTF((font_path / "OpenSans-Regular.ttf").string().c_str(), g_font_size_header_2);
		m_fonts[font_type::header_1] =		io.Fonts->AddFontFromFileTTF((font_path / "OpenSans-Regular.ttf").string().c_str(), g_font_size_header_1);
		m_fonts[font_type::header_2] =		io.Fonts->AddFontFromFileTTF((font_path / "OpenSans-Regular.ttf").string().c_str(), g_font_size_header_0);

		m_fonts[font_type::giant] =			io.Fonts->AddFontFromFileTTF((font_path / "OpenSans-Bold.ttf").string().c_str(), 60.f);

		//Inconsolata-Regular
		m_fonts[font_type::monospace_regular] 		= io.Fonts->AddFontFromFileTTF((Inconsolata_path / "Inconsolata-Regular.ttf").string().c_str(), g_font_size * 0.92f);
		m_fonts[font_type::monospace_regular_big] 	= io.Fonts->AddFontFromFileTTF((Inconsolata_path / "Inconsolata-Regular.ttf").string().c_str(), g_big_font_size * 1.92f);

		io.FontDefault = m_fonts[font_type::regular];

		application::get().get_renderer()->imgui_create_fonts();
	}


	void imgui_config::resize_fonts(const f32 font_size) {

		g_font_size = font_size;
		g_big_font_size = font_size * 1.2f;
		g_font_size_header_0 = font_size * 	1.2666666666f;
		g_font_size_header_1 = font_size * 	1.5333333333f;
		g_font_size_header_2 = font_size * 	1.8f;

		load_fonts();
	}

	void imgui_config::resize_fonts(const f32 font_size, const f32 big_font_size, const f32 font_size_header_0, const f32 font_size_header_1, const f32 font_size_header_2) {

		g_font_size = font_size;
		g_big_font_size = big_font_size;
		g_font_size_header_0 = font_size_header_0;
		g_font_size_header_1 = font_size_header_1;
		g_font_size_header_2 = font_size_header_2;

		load_fonts();
	}


	imgui_config::imgui_config() { 
			
		IMGUI_CHECKVERSION();
		m_context_imgui = ImGui::CreateContext();
		m_context_implot = ImPlot::CreateContext();
		application::get().get_renderer()->imgui_init();
		

		main_color = { .0f,	.4088f,	1.0f,	1.f };
		g_window_border = false;
		g_highlighted_window_bg = LERP_GRAY(0.57f);

		serialize(serializer::option::load_from_file);

		main_titlebar_color = LERP_MAIN_COLOR_DARK(.5f);			// lerp after loading main color
		action_color_00_faded = LERP_MAIN_COLOR_DARK(0.5f);
		action_color_00_weak = LERP_MAIN_COLOR_DARK(0.6f);
		action_color_00_default = LERP_MAIN_COLOR_DARK(0.7f);
		action_color_00_hover = LERP_MAIN_COLOR_DARK(0.85f);
		action_color_00_active = LERP_MAIN_COLOR_DARK(1.f);
	
		load_fonts();
		update_UI_theme();

		LOG_INIT 
	}


	imgui_config::~imgui_config() { 
				
		application::get().get_renderer()->imgui_shutdown();
		ImGui::DestroyContext(m_context_imgui);
		ImPlot::DestroyContext(m_context_implot);

		serialize(serializer::option::save_to_file);
		LOG_SHUTDOWN
	}


	ImFont* imgui_config::get_font(const font_type type) {

		if (m_fonts.contains(type))
			return m_fonts.at(type);
			
		return nullptr;
	}


	void imgui_config::serialize(serializer::option option) {

		AT::serializer::yaml(config::get_filepath_from_configtype(util::get_executable_path(), config::file::ui), "theme", option)			// load general appearance settings
			.entry(KEY_VALUE(g_font_size))
			.entry(KEY_VALUE(g_font_size_header_0))
			.entry(KEY_VALUE(g_font_size_header_1))
			.entry(KEY_VALUE(g_font_size_header_2))
			.entry(KEY_VALUE(g_big_font_size))
			.entry(KEY_VALUE(g_UI_theme))
			.entry(KEY_VALUE(g_window_border))
			.entry(KEY_VALUE(g_highlighted_window_bg))

			// color
			.entry(KEY_VALUE(main_color))
			.entry(KEY_VALUE(main_titlebar_color))

			// gray
			.entry(KEY_VALUE(action_color_gray_default))
			.entry(KEY_VALUE(action_color_gray_hover))
			.entry(KEY_VALUE(action_color_gray_active));
	}

	// --------------------------------------------------------------------------------------------------------------------------------------------

	void update_UI_colors(ImVec4 new_color) {

		main_color = new_color;
		application::get().get_imgui_config_ref()->serialize(serializer::option::save_to_file);
		update_UI_theme();
	}


	void enable_window_border(bool enable) {

		g_window_border = enable;
		//serialize(serializer::option::save_to_file);

		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowBorderSize = enable ? 1.f : 0.f;
	}
	

	void update_UI_theme() {
		
		//LOG(Debug, "updating UI theme");

		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		// main sizes
		style->WindowPadding = ImVec2(10.f, 4.f);
		style->FramePadding = ImVec2(4.f, 4.f);
		style->CellPadding = ImVec2(4.f, 4.f);
		style->ItemSpacing = ImVec2(4.f, 4.f);
		style->ItemInnerSpacing = ImVec2(4.f, 4.f);
		style->TouchExtraPadding = ImVec2(4.f, 4.f);
		style->IndentSpacing = 10.f;
		style->ScrollbarSize = 14.f;
		style->GrabMinSize = 14.f;
		style->WindowMenuButtonPosition = ImGuiDir_Right;

		// border
		style->WindowBorderSize = 1.0f;
		style->ChildBorderSize = 0.0f;
		style->PopupBorderSize = 0.0f;
		style->FrameBorderSize = 0.0f;
		style->TabBorderSize = 0.0f;
		style->TabBarBorderSize = 0.0f;

		// padding
		style->WindowRounding = 2.f;
		style->ChildRounding = 2.f;
		style->FrameRounding = 2.f;
		style->PopupRounding = 2.f;
		style->ScrollbarRounding = 2.f;
		style->GrabRounding = 2.f;
		style->TabRounding = 2.f;

		switch (g_UI_theme) {

			case AT::UI::theme_selection::dark: {

				action_color_00_faded = LERP_MAIN_COLOR_DARK(0.5f);
				action_color_00_weak = LERP_MAIN_COLOR_DARK(0.6f);
				action_color_00_default = LERP_MAIN_COLOR_DARK(0.7f);
				action_color_00_hover = LERP_MAIN_COLOR_DARK(0.85f);
				action_color_00_active = LERP_MAIN_COLOR_DARK(0.92f);

				action_color_gray_default = LERP_GRAY(0.15f);
				action_color_gray_hover = LERP_GRAY(0.2f);
				action_color_gray_active = LERP_GRAY(0.25f);

				g_highlighted_window_bg = LERP_GRAY(0.57f);
				main_titlebar_color = LERP_MAIN_COLOR_DARK(.5f);

				colors[ImGuiCol_Text]					= IMCOLOR_GRAY(255);
				colors[ImGuiCol_TextDisabled]			= IMCOLOR_GRAY(180);
				colors[ImGuiCol_WindowBg]				= default_gray;
				colors[ImGuiCol_ChildBg]				= default_gray;
				colors[ImGuiCol_PopupBg]				= IMCOLOR_GRAY(20);
				colors[ImGuiCol_Border]					= LERP_GRAY_A(.43f, .5f);
				colors[ImGuiCol_BorderShadow]			= LERP_GRAY_A(.12f, .5f);
				colors[ImGuiCol_FrameBg]				= LERP_GRAY_A(.06f, .54f);
				colors[ImGuiCol_FrameBgHovered]			= LERP_GRAY_A(.19f, .4f);
				colors[ImGuiCol_FrameBgActive]			= LERP_GRAY_A(.3f, .67f);
				colors[ImGuiCol_TitleBg]				= IMCOLOR_GRAY(22);
				colors[ImGuiCol_TitleBgActive]			= IMCOLOR_GRAY(22);
				colors[ImGuiCol_TitleBgCollapsed]		= IMCOLOR_GRAY(22);
				colors[ImGuiCol_MenuBarBg]				= LERP_GRAY(.1f);
				colors[ImGuiCol_ScrollbarBg]			= LERP_GRAY(0.23f);
				colors[ImGuiCol_ScrollbarGrab]			= action_color_00_default;
				colors[ImGuiCol_ScrollbarGrabHovered]	= action_color_00_hover;
				colors[ImGuiCol_ScrollbarGrabActive]	= action_color_00_active;
				colors[ImGuiCol_CheckMark]				= action_color_00_active;
				colors[ImGuiCol_SliderGrab]				= action_color_00_default;
				colors[ImGuiCol_SliderGrabActive]		= action_color_00_active;
				colors[ImGuiCol_Button]					= action_color_00_default;
				colors[ImGuiCol_ButtonHovered]			= action_color_00_hover;
				colors[ImGuiCol_ButtonActive]			= action_color_00_active;
				colors[ImGuiCol_Header]					= LERP_GRAY(.3f);
				colors[ImGuiCol_HeaderHovered]			= LERP_GRAY(.4f);
				colors[ImGuiCol_HeaderActive]			= LERP_GRAY(.5f);
				colors[ImGuiCol_Separator]				= LERP_GRAY(.45f);
				colors[ImGuiCol_SeparatorHovered]		= LERP_GRAY(.45f);
				colors[ImGuiCol_SeparatorActive]		= LERP_GRAY(.45f);
				colors[ImGuiCol_ResizeGrip]				= action_color_00_default;
				colors[ImGuiCol_ResizeGripHovered]		= action_color_00_hover;
				colors[ImGuiCol_ResizeGripActive]		= action_color_00_active;
				colors[ImGuiCol_Tab]					= LERP_MAIN_COLOR_DARK(0.4f);
				colors[ImGuiCol_TabHovered]				= LERP_MAIN_COLOR_DARK(0.5f);
				colors[ImGuiCol_TabActive]				= LERP_MAIN_COLOR_DARK(0.6f);
				colors[ImGuiCol_TabUnfocused]			= LERP_MAIN_COLOR_DARK(0.5f);
				colors[ImGuiCol_TabUnfocusedActive]		= LERP_MAIN_COLOR_DARK(0.6f);
				colors[ImGuiCol_DockingPreview]			= action_color_00_active;
				colors[ImGuiCol_DockingEmptyBg]			= LERP_GRAY(0.2f);
				colors[ImGuiCol_PlotLines]				= ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
				colors[ImGuiCol_PlotLinesHovered]		= ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
				colors[ImGuiCol_PlotHistogram]			= LERP_MAIN_COLOR_DARK(.75f);
				colors[ImGuiCol_PlotHistogramHovered]	= action_color_00_active;
				colors[ImGuiCol_TableHeaderBg]			= ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
				colors[ImGuiCol_TableBorderStrong]		= ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
				colors[ImGuiCol_TableBorderLight]		= ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
				colors[ImGuiCol_TableRowBg]				= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
				colors[ImGuiCol_TableRowBgAlt]			= ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
				colors[ImGuiCol_TextSelectedBg]			= LERP_MAIN_COLOR_DARK(.4f);
				colors[ImGuiCol_DragDropTarget]			= LERP_MAIN_COLOR_DARK(.6f);
				colors[ImGuiCol_NavHighlight]			= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
				colors[ImGuiCol_NavWindowingHighlight]	= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
				colors[ImGuiCol_NavWindowingDimBg]		= ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
				colors[ImGuiCol_ModalWindowDimBg]		= ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

			} break;

			case AT::UI::theme_selection::light: {

				action_color_00_default = LERP_MAIN_COLOR_LIGHT(0.7f);
				action_color_00_hover = LERP_MAIN_COLOR_LIGHT(0.85f);
				action_color_00_active = LERP_MAIN_COLOR_LIGHT(1.f);

				action_color_gray_default = LERP_GRAY(0.2f);
				action_color_gray_hover = LERP_GRAY(0.27f);
				action_color_gray_active = LERP_GRAY(0.35f);

				g_highlighted_window_bg = LERP_GRAY(0.8f);
				main_titlebar_color = LERP_MAIN_COLOR_LIGHT(.5f);

				colors[ImGuiCol_Text]					= LERP_GRAY(1.0f);
				colors[ImGuiCol_TextDisabled]			= LERP_GRAY(.7f);
				colors[ImGuiCol_WindowBg]				= LERP_GRAY(.25f);
				colors[ImGuiCol_ChildBg]				= LERP_GRAY(.25f);
				colors[ImGuiCol_PopupBg]				= LERP_GRAY(.25f);
				colors[ImGuiCol_Border]					= LERP_GRAY_A(0.2f, .50f);
				colors[ImGuiCol_BorderShadow]			= LERP_GRAY(.12f);
				colors[ImGuiCol_FrameBg]				= LERP_GRAY_A(.75f, .75f);
				colors[ImGuiCol_FrameBgHovered]			= LERP_GRAY_A(.70f, .75f);
				colors[ImGuiCol_FrameBgActive]			= LERP_GRAY_A(.65f, .75f);
				colors[ImGuiCol_TitleBg]				= action_color_00_default;
				colors[ImGuiCol_TitleBgActive]			= action_color_00_active;
				colors[ImGuiCol_TitleBgCollapsed]		= action_color_00_default;
				colors[ImGuiCol_MenuBarBg]				= LERP_GRAY(.58f);
				colors[ImGuiCol_ScrollbarBg]			= LERP_GRAY(.75f);
				colors[ImGuiCol_ScrollbarGrab]			= action_color_00_default;
				colors[ImGuiCol_ScrollbarGrabHovered]	= action_color_00_hover;
				colors[ImGuiCol_ScrollbarGrabActive]	= action_color_00_active;
				colors[ImGuiCol_CheckMark]				= action_color_00_active;
				colors[ImGuiCol_SliderGrab]				= action_color_00_default;
				colors[ImGuiCol_SliderGrabActive]		= action_color_00_active;
				colors[ImGuiCol_Button]					= action_color_00_default;
				colors[ImGuiCol_ButtonHovered]			= action_color_00_hover;
				colors[ImGuiCol_ButtonActive]			= action_color_00_active;
				colors[ImGuiCol_Header]					= LERP_GRAY(.75f);
				colors[ImGuiCol_HeaderHovered]			= LERP_GRAY(.7f);
				colors[ImGuiCol_HeaderActive]			= LERP_GRAY(.65f);
				colors[ImGuiCol_Separator]				= LERP_GRAY(.45f);
				colors[ImGuiCol_SeparatorHovered]		= LERP_GRAY(.45f);
				colors[ImGuiCol_SeparatorActive]		= LERP_GRAY(.45f);
				colors[ImGuiCol_ResizeGrip]				= action_color_00_default;
				colors[ImGuiCol_ResizeGripHovered]		= action_color_00_hover;
				colors[ImGuiCol_ResizeGripActive]		= action_color_00_active;
				colors[ImGuiCol_Tab]					= action_color_00_default;
				colors[ImGuiCol_TabHovered]				= action_color_00_hover;
				colors[ImGuiCol_TabActive]				= action_color_00_active;
				colors[ImGuiCol_TabUnfocused]			= LERP_MAIN_COLOR_LIGHT(0.5f);
				colors[ImGuiCol_TabUnfocusedActive]		= LERP_MAIN_COLOR_LIGHT(0.6f);
				colors[ImGuiCol_DockingPreview]			= action_color_00_active;
				colors[ImGuiCol_DockingEmptyBg]			= LERP_GRAY(.2f);
				colors[ImGuiCol_PlotLines]				= ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
				colors[ImGuiCol_PlotLinesHovered]		= ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
				colors[ImGuiCol_PlotHistogram]			= LERP_MAIN_COLOR_LIGHT(.75f);
				colors[ImGuiCol_PlotHistogramHovered]	= action_color_00_active;
				colors[ImGuiCol_TableHeaderBg]			= ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
				colors[ImGuiCol_TableBorderStrong]		= ImVec4(0.31f, 0.31f, 0.35f, 1.00f);	// Prefer using Alpha=1.0 here
				colors[ImGuiCol_TableBorderLight]		= ImVec4(0.23f, 0.23f, 0.25f, 1.00f);	// Prefer using Alpha=1.0 here
				colors[ImGuiCol_TableRowBg]				= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
				colors[ImGuiCol_TableRowBgAlt]			= ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
				colors[ImGuiCol_TextSelectedBg]			= LERP_MAIN_COLOR_LIGHT(.4f);
				colors[ImGuiCol_DragDropTarget]			= ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
				colors[ImGuiCol_NavHighlight]			= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
				colors[ImGuiCol_NavWindowingHighlight]	= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
				colors[ImGuiCol_NavWindowingDimBg]		= ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
				colors[ImGuiCol_ModalWindowDimBg]		= ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

			} break;

			default:
				break;
		}

	}

}
