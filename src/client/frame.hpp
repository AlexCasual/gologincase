#pragma once
#include <wx/wx.h>
#include <tcp/client/client.hpp>

namespace gologin
{	
	namespace gui
	{
		using frame_handlers_t = struct
		{
			std::function<bool(const wxString&, wxString&)> _on_login;
			std::function<bool(wxString&)> _on_logout;
			std::function<bool(const wxString&, wxString&)> _on_connect;
			std::function<bool(wxString&)> _on_disconnect;
			std::function<bool(const wxString&, wxString&)> _on_send;
		};
		
		class frame : public wxFrame
		{
		public:
			frame(const wxString& title, const frame_handlers_t& handlers);
			~frame() override;

			void on_about(wxCommandEvent& event);
			void on_quit(wxCommandEvent& event);
			void on_login(wxCommandEvent& event);
			void on_logout(wxCommandEvent& event);
			void on_size(wxSizeEvent& event);
			void on_disconnect(wxCommandEvent& event);
			void on_connect(wxCommandEvent& event);
			void on_send(wxCommandEvent& event);
			
			void view_message(const wxString& msg);

		public:
			void load_icon();
			
		private:
			wxString m_title;
			frame_handlers_t m_handlers;
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