
#pragma once

// FORWARD DECLARATIONS ================================================================================================

namespace GLT { class event; }


namespace GLT::UI {

	// CONSTANTS =======================================================================================================

	// MACROS ==========================================================================================================

	// TYPES ===========================================================================================================

	// STATIC VARIABLES ================================================================================================

	// FUNCTION DECLARATION ============================================================================================

	// TEMPLATE DECLARATION ============================================================================================

	// CLASS DECLARATION ===============================================================================================

	class editor_window {
	public:

		editor_window() {};
		virtual ~editor_window() {};

		DEFAULT_GETTER_C(std::string, 		window_name)

		virtual void window() {};
		virtual void update(const f32 delta_time) {};
		virtual void on_event(GLT::event& event) {};

		void focus_window();
		FORCEINLINE bool should_close() const { return !m_show_window; }
		FORCEINLINE virtual void show_possible_sub_window_options() {}

	protected:

		void make_window_name(const char* base_name);
		std::string 			m_window_name{};
		bool 					m_show_window = true;		// set to false to close window
		bool 					m_window_visible = true;	// can be used to optimize performance
        UUID					m_ID{};

	};

}
