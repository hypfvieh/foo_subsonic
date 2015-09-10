#include "foo_subsonic.h"

class input_subsonic
{
	bool no_loop, eof;

	service_ptr_t<file> m_file;

	pfc::string8 m_path;

	int err;

	int data_written, remainder, pos_delta, startsilence, silence;

	double qsfemu_pos;

	int song_len, fade_len;
	int tag_song_ms, tag_fade_ms;

	file_info_impl m_info;

	bool do_filter, do_suppressendsilence;

public:

	~input_subsonic()
	{
		
	}

	void open(service_ptr_t<file> p_file, const char * p_path, t_input_open_reason p_reason, abort_callback & p_abort)
	{
		// prepare http connection
	}

	void get_info(file_info & p_info, abort_callback & p_abort)
	{
		p_info.copy(m_info);
	}

	t_filestats get_file_stats(abort_callback & p_abort)
	{
		return m_file->get_stats(p_abort);
	}

	void decode_initialize(unsigned p_flags, abort_callback & p_abort)
	{
		
	}

	bool decode_run(audio_chunk & p_chunk, abort_callback & p_abort)
	{
		// 
		return true;
	}

	void decode_seek(double p_seconds, abort_callback & p_abort)
	{
		
	}

	bool decode_can_seek()
	{
		return true;
	}

	bool decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta)
	{
		return false;
	}

	bool decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta)
	{
		return false;
	}

	void decode_on_idle(abort_callback & p_abort)
	{
	}

	void retag(const file_info & p_info, abort_callback & p_abort)
	{
		// unsupported
	}

	static bool g_is_our_content_type(const char * p_content_type)
	{
		// TODO: Check for content type
		return false;
	}

	static bool g_is_our_path(const char * p_full_path, const char * p_extension)
	{
		// TODO: check if this is http/https url
		return (!stricmp(p_extension, "qsf") || !stricmp(p_extension, "miniqsf"));
	}

private:

};


static input_singletrack_factory_t<input_subsonic>               g_input_subsonic_factory;

