#include "app.hpp"
#include <messages/messages.hpp>
#include <messages/json/packer.hpp>
#include <tcp/core/dispatcher.hpp>

namespace gologin::gui
{
	BEGIN_EVENT_TABLE(frame, wxFrame)
		EVT_MENU(wxID_ABOUT, frame::on_about)
		EVT_MENU(wxID_EXIT, frame::on_quit)
		EVT_SIZE(frame::on_size)
		END_EVENT_TABLE()

	namespace detail
	{
		static const wxSize min_frame_size = { 350,350 };
		static const wxSize max_frame_size = { 350,350 };
		static const wxSize min_bottom_size = { 350,350 };

		static const char default_address[] = "localhost:10500";
		const char default_login[] = "login:password";
		static const char connect_button[] = "Connect";
		static const char disconnect_button[] = "Disconnect";
		static const char login_button[] = "Login";
		static const char logout_button[] = "Logout";
		static const char send_button[] = "Send";
		static const char sender_prefix[] = "YOU : ";
		static const char icon_file[] = "\\icon\\icon.bmp";
	}

	frame::frame(const wxString& title, const input_handlers_t& input, ouput_handlers_t& output)
		: wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
		,  m_title(title), m_input_handlers(input)
	{
		output_hadnlers(output);

		//load_icon();

		load_ui();
	}

	frame::~frame()
	{
		m_main_panel;
	}

	void frame::on_size(wxSizeEvent& event)
	{
		event.Skip();
	}

	void frame::on_quit(wxCommandEvent& event)
	{
		Close(true);
	}

	void frame::on_connect(wxCommandEvent& event)
	{
		auto _address = m_address_field->GetValue();
		if (_address.empty())
		{
			m_status_bar->SetStatusText(_T("Invalid server address."));
			return;
		}

		wxString _text; 
		ui_status _status;
		input_handler(ui_action::on_connect, _address);
	}

	void frame::on_disconnect(wxCommandEvent& event)
	{
		input_handler(ui_action::on_disconnect, wxString{});
	}
	
	void frame::on_login(wxCommandEvent& event)
	{
		auto _userpass = m_login_field->GetValue();
		if (_userpass.empty())
		{
			m_status_bar->SetStatusText(_T("Invalid login or password."));
			return;
		}

		input_handler(ui_action::on_login, _userpass);
	}

	void frame::on_logout(wxCommandEvent& event)
	{
		input_handler(ui_action::on_logout, wxString{});
	}

	void frame::on_send(wxCommandEvent& event)
	{
		auto _msg = m_input_field->GetValue();

		if (_msg.empty())
		{
			m_status_bar->SetStatusText(_T("Invalid message."));
			return;
		}

		input_handler(ui_action::on_send, _msg);
	}

	void frame::on_about(wxCommandEvent& event)
	{
		wxMessageBox(_T("Gologin test case by a.zawadski [2021]"));
	}

	void frame::load_ui()
	{
		this->Center();

		this->SetMinSize(detail::min_frame_size);
		this->SetMaxSize(detail::max_frame_size);

		//status bar
		{
			m_status_bar = CreateStatusBar();

			int bar_widths[] = { -1 };
			m_status_bar->SetStatusWidths(_countof(bar_widths), bar_widths);

			int bar_styles[] = { wxSB_FLAT };
			m_status_bar->SetStatusStyles(_countof(bar_styles), bar_styles);

			m_status_bar->SetStatusText(_T("Not connected."));
		}

		// main panel
		{
			m_main_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
			m_main_frame = new wxBoxSizer(wxVERTICAL);

			m_main_panel->SetSizer(m_main_frame);
		}

		// controls
		{
			m_address_field = new wxTextCtrl(m_main_panel, wxID_ANY, detail::default_address, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
			m_login_field = new wxTextCtrl(m_main_panel, wxID_ANY, detail::default_login, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

			m_input_field = new wxTextCtrl(m_main_panel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTEXT_TYPE_ANY);
			m_output_field = new wxTextCtrl(m_main_panel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

			m_connect_button = new wxButton(m_main_panel, wxID_ANY, detail::connect_button);
			m_disconnect_button = new wxButton(m_main_panel, wxID_ANY, detail::disconnect_button);
			
			m_login_button = new wxButton(m_main_panel, wxID_ANY, detail::login_button);
			m_logout_button = new wxButton(m_main_panel, wxID_ANY, detail::logout_button);
			
			m_send_button = new wxButton(m_main_panel, wxID_ANY, detail::send_button);

			m_logout_button->Hide();
			m_disconnect_button->Hide();
			m_send_button->Disable();
			m_login_button->Disable();
			m_login_field->Disable();
			m_input_field->Disable();
		}

		wxBoxSizer* _address_frame = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* _login_frame = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* _input_frame = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* _output_frame = new wxBoxSizer(wxVERTICAL);

		//address
		{
			_address_frame->Add(m_address_field, 1, wxALL);
			_address_frame->Add(m_connect_button, 0, wxALL | wxEXPAND);
			_address_frame->Add(m_disconnect_button, 0, wxALL | wxEXPAND);

			_address_frame->AddStretchSpacer();

			m_main_frame->Add(_address_frame, 0, wxEXPAND);
		}
		
		//login
		{
			_login_frame->Add(m_login_field, 1, wxALL);
			_login_frame->Add(m_login_button, 0, wxALL | wxEXPAND);
			_login_frame->Add(m_logout_button, 0, wxALL | wxEXPAND);

			_login_frame->AddStretchSpacer();

			m_main_frame->Add(_login_frame, 0, wxEXPAND);
		}

		//input
		{
			_input_frame->Add(m_input_field, 1, wxALL | wxALIGN_CENTRE);
			_input_frame->Add(m_send_button, 0, wxALIGN_BOTTOM);

			_input_frame->AddStretchSpacer();

			m_main_frame->Add(_input_frame, 0, wxEXPAND);
		}

		//output
		{
			_output_frame->SetMinSize(detail::min_bottom_size);
			_output_frame->Add(m_output_field, 1, wxALL | wxEXPAND);

			m_main_frame->Add(_output_frame, 0, wxEXPAND);
		}

		m_main_frame->FitInside(this);
		m_main_panel->Layout();

		Connect(m_connect_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(frame::on_connect));
		Connect(m_disconnect_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(frame::on_disconnect));
		Connect(m_login_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(frame::on_login));
		Connect(m_logout_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(frame::on_logout));
		Connect(m_send_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(frame::on_send));
	}
	
	void frame::load_icon()
	{
		auto _app_dir = wxGetCwd();
		auto _home_dir = wxGetHomeDir();

		_app_dir += detail::icon_file;

		wxIcon _icon(wxICON(green_simple_crest_xpm));

		//_icon.LoadFile(_app_dir);

		this->SetIcon(_icon);
	}

	void frame::input_handler(const ui_action& act, const wxString& data)
	{
		if (auto _h = m_input_handlers.find(act); _h != m_input_handlers.end())
		{
			return _h->second(data);
		}
	}

	void frame::output_hadnlers(ouput_handlers_t& handlers)
	{
		handlers[ui_action::on_login] = [&](const ui_status& st, const wxString& msg, const wxString& data)
			{
				return on_login(st, msg, data);
			};

		handlers[ui_action::on_logout] = [&](const ui_status& st, const wxString& msg, const wxString& data)
			{
				return on_logout(st, msg, data);
			};

		handlers[ui_action::on_connect] = [&](const ui_status& st, const wxString& msg, const wxString& data)
			{
				return on_connect(st, msg, data);
			};

		handlers[ui_action::on_disconnect] = [&](const ui_status& st, const wxString& msg, const wxString& data)
			{
				return on_disconnect(st, msg, data);
			};

		handlers[ui_action::on_recv] = [&](const ui_status& st, const wxString& msg, const wxString& data)
			{
				return on_recv(st, msg, data);
			};

		handlers[ui_action::on_send] = [&](const ui_status& st, const wxString& msg, const wxString& data)
			{
				return on_send(st, msg, data);
			};
	}

	void frame::on_connect(const ui_status& st, const wxString& msg, const wxString& data)
	{
		if (st == ui_status::success)
		{
			m_disconnect_button->Show(true);

			m_address_field->Disable();

			if(!m_login_button->IsEnabled())
			{
				m_login_button->Enable();
				m_login_field->Enable();
			}

			if(!m_logout_button->IsEnabled())
			{
				m_logout_button->Enable();
				m_login_field->Enable();
			}

			m_connect_button->Hide();
		}
		else if (st == ui_status::pending)
		{
			m_address_field->Disable();
			m_connect_button->Disable();
		}
		
		m_status_bar->SetStatusText(msg);

		m_main_panel->Layout();
	}

	void frame::on_disconnect(const ui_status& st, const wxString& msg, const wxString& data)
	{
		if (st == ui_status::success)
		{
			m_connect_button->Show(true);

			m_connect_button->Enable();

			m_address_field->Enable();

			m_disconnect_button->Hide();

			if (m_send_button->IsEnabled())
			{
				m_send_button->Disable();
				m_input_field->Disable();
			}

			if (m_login_button->IsEnabled())
			{
				m_login_button->Disable();
				m_login_field->Disable();
			}

			m_logout_button->Hide();
			m_login_button->Show(true);
			m_login_field->Disable();
		}
		else if (st == ui_status::pending)
		{
			m_disconnect_button->Disable();
			m_main_panel->Layout();
		}
		
		m_status_bar->SetStatusText(msg);

		m_main_panel->Layout();
	}

	void frame::on_login(const ui_status& st, const wxString& msg, const wxString& data)
	{
		if (st == ui_status::success)
		{		
			m_logout_button->Show(true);

			m_logout_button->Enable();

			m_login_field->Disable();

			m_login_button->Hide();

			m_send_button->Enable();
			m_input_field->Enable();
		}
		else if (st == ui_status::pending)
		{
			m_login_button->Disable();
			m_main_panel->Layout();
		}

		m_status_bar->SetStatusText(msg);

		m_main_panel->Layout();
	}

	void frame::on_logout(const ui_status& st, const wxString& msg, const wxString& data)
	{
		if (st == ui_status::success)
		{
			m_login_button->Show(true);

			m_login_field->Enable();

			m_logout_button->Hide();
			
			m_send_button->Disable();
			m_input_field->Disable();
		}
		else if (st == ui_status::pending)
		{
			m_logout_button->Disable();
		}

		m_status_bar->SetStatusText(msg);

		m_main_panel->Layout();
	}

	void frame::on_send(const ui_status& st, const wxString& msg, const wxString& data)
	{
		if (st == ui_status::pending)
		{
			m_input_field->Clear();
			
			m_status_bar->SetStatusText(msg);
			m_main_panel->Layout();
		}
	}

	void frame::on_recv(const ui_status& st, const wxString& msg, const wxString& data)
	{
		if (st == ui_status::success)
		{
			m_output_field->AppendText(msg);
			m_output_field->AppendText(_T(": "));
			m_output_field->AppendText(data);

			m_output_field->SetInsertionPointEnd();
			m_output_field->AppendText(_T("\n"));

			m_main_panel->Layout();
		}
	}
}