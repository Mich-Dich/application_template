
#include "util/pch_editor.h"

#include "ui/util/editor_window.h"

namespace GLT::UI {

	void editor_window::make_window_name(const char* base_name) {

		std::stringstream ss;
		ss << base_name << "##" << m_ID;
		m_window_name = ss.str();
	}

	void editor_window::focus_window() {

		if (!m_window_name.empty()) {
            ImGui::SetWindowFocus(m_window_name.c_str());
        }
	}

}
