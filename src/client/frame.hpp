#pragma once
#include <wx/wx.h>
#include <tcp/client/client.hpp>

namespace gologin
{	
	namespace gui
	{
		enum class ui_status
		{
			success = 1,
			failed,
			pending
		};

		enum class ui_action
		{
			on_login = 1, 
			on_logout,
			on_connect,
			on_disconnect,
			on_send,
			on_recv
		};

		using input_handler_t = std::function<void(const wxString&)>;
		using ouput_handler_t = std::function<void(const ui_status&, const wxString&, const wxString&)>;

		using input_handlers_t = std::unordered_map<ui_action, input_handler_t>;
		using ouput_handlers_t = std::unordered_map<ui_action, ouput_handler_t>;
		
		class frame : public wxFrame
		{
		public:
			frame(const wxString& title, const input_handlers_t& input, ouput_handlers_t& output);
			~frame() override;

			void on_size(wxSizeEvent& event);
			void on_about(wxCommandEvent& event);
			void on_quit(wxCommandEvent& event);
			void on_login(wxCommandEvent& event);
			void on_logout(wxCommandEvent& event);
			void on_disconnect(wxCommandEvent& event);
			void on_connect(wxCommandEvent& event);
			void on_send(wxCommandEvent& event);

	private:
			void load_ui();
			void load_icon();
			void output_hadnlers(ouput_handlers_t& handlers);
			void input_handler(const ui_action& act, const wxString& data);
			void on_login(const ui_status& st, const wxString& msg, const wxString& data);
			void on_logout(const ui_status& st, const wxString& msg, const wxString& data);
			void on_connect(const ui_status& st, const wxString& msg, const wxString& data);
			void on_disconnect(const ui_status& st, const wxString& msg, const wxString& data);
			void on_send(const ui_status& st, const wxString& msg, const wxString& data);
			void on_recv(const ui_status& st, const wxString& msg, const wxString& data);

		private:
			wxString m_title;
			input_handlers_t m_input_handlers;
			wxButton* m_connect_button;
			wxButton* m_disconnect_button;
			wxButton* m_login_button;
			wxButton* m_logout_button;
			wxButton* m_send_button;
			wxTextCtrl* m_input_field;
			wxTextCtrl* m_address_field;
			wxTextCtrl* m_login_field;
			wxTextCtrl* m_output_field;
			wxStatusBar* m_status_bar;
			wxPanel* m_main_panel;
			wxBoxSizer* m_main_frame;
			DECLARE_EVENT_TABLE()
		};

		enum
		{
			ID_BUTTON = 108,
			ID_LIST
		};
	}
}