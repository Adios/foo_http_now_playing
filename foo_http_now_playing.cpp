#include "foobar2000/SDK/foobar2000.h"
#include "foobar2000/helpers/helpers.h"

#include "resource.h"

using namespace pfc;

DECLARE_COMPONENT_VERSION(
	"HTTP Now Playing",
	"0.1",
	"Make an HTTP request to remote server.\n"
	"Source: http://github.com/Adios/http_now_playing"
);

/*
 *
 *  Preferences Page.
 *
 */

// c6d0c8dc-54f2-4ff2-8ca8-a45bcf5bb316
static const GUID preference_guid = { 0xc6d0c8dc, 0x54f2, 0x4ff2, { 0x8c, 0xa8, 0xa4, 0x5b, 0xcf, 0x5b, 0xb3, 0x16 } };
// b48860e3-7aec-4fd4-8796-0dfecbfc5fb8
static const GUID conf_url_guid = { 0xb48860e3, 0x7aec, 0x4fd4, { 0x87, 0x96, 0x0d, 0xfe, 0xcb, 0xfc, 0x5f, 0xb8 } };
static cfg_string conf_url(conf_url_guid, "");

static BOOL CALLBACK dialog(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		uSetDlgItemText(hwnd, IDC_URL, conf_url);
		break;
	case WM_COMMAND:
		if (wp == (EN_KILLFOCUS << 16 | IDC_URL))
			conf_url = string_utf8_from_window( (HWND)lp );
		break;
	default:
		return 0;
	}
	return 1;
}

class now_playing_page : public preferences_page_instance
{
	HWND hwnd;
	t_uint32 state;
public:
	now_playing_page(HWND parent, preferences_page_callback::ptr callback)
	{
		state = preferences_state::resettable;
		hwnd = uCreateDialog(IDD_CONFIG, parent, dialog, 0);
	}

	void apply() {}
	void reset() {}
	HWND get_wnd() { return hwnd; }
	t_uint32 get_state() { return state; }
};

class now_playing_conf : public preferences_page_v3
{
	const char *get_name() { return "HTTP Now Playing"; }
	GUID get_parent_guid() { return guid_display; }
	GUID get_guid() { return preference_guid; }

	preferences_page_instance::ptr instantiate(HWND parent, preferences_page_callback::ptr callback) {
		return new service_impl_t<now_playing_page>(parent, callback);
	}
};

/*
 *
 *  Playback callback.
 *
 */

static abort_callback_dummy dont_care;

class now_playing_callback : public play_callback_static
{
	unsigned get_flags() { return flag_on_playback_new_track | flag_on_playback_stop; }
	void on_playback_seek(double) {}
	void on_playback_pause(bool) {}
	void on_playback_edited(metadb_handle_ptr) {}
	void on_playback_dynamic_info(const file_info&) {}
	void on_playback_time(double) {}
	void on_volume_change(float) {}
	void on_playback_dynamic_info_track(const file_info&) {}
	void on_playback_starting(play_control::t_track_command, bool) {}
	void on_playback_stop(play_control::t_stop_reason) {}

	void now_playing_callback::on_playback_new_track(metadb_handle_ptr track)
	{
		service_ptr_t<titleformat_object> script;
		static_api_ptr_t<playback_control> pc;
		static_api_ptr_t<playlist_manager> pm;
		static_api_ptr_t<http_client> http;
		http_request_post::ptr req;
		string8 buf;

		http->create_request("POST")->service_query_t(req);

		static_api_ptr_t<titleformat_compiler>()->compile_safe(script, "%album%");
		pc->playback_format_title_ex(track, NULL, buf, script, NULL, play_control::display_level_titles);
		req->add_post_data("album", buf);

		static_api_ptr_t<titleformat_compiler>()->compile_safe(script, "%artist%");
		pc->playback_format_title_ex(track, NULL, buf, script, NULL, play_control::display_level_titles);
		req->add_post_data("artist", buf);
		
		static_api_ptr_t<titleformat_compiler>()->compile_safe(script, "%title%");
		pc->playback_format_title_ex(track, NULL, buf, script, NULL, play_control::display_level_titles);
		req->add_post_data("title", buf);

		static_api_ptr_t<titleformat_compiler>()->compile_safe(script, "%filename_ext%");
		pc->playback_format_title_ex(track, NULL, buf, script, NULL, play_control::display_level_titles);
		req->add_post_data("filename_ext", buf);

		pm->playlist_get_name(pm->get_playing_playlist(), buf);
		req->add_post_data("playlist", buf);

		try { req->run_ex(conf_url, dont_care); } 
		catch (exception) { console::info("HTTP Now Playing: URL is invalid."); }
	}
};

static preferences_page_factory_t<now_playing_conf> playing_conf_factory;
static play_callback_static_factory_t<now_playing_callback> playing_callback_factory;