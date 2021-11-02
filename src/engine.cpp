#include <string>

#include "config.hpp"
#include "engine.hpp"
#include "event.hpp"

Engine::Engine(int n_loops, const char* osc_in_port)
{
    m_osc_port = osc_in_port;
    m_n_loops = n_loops;

    midi_init();
    osc_init();

    m_bpm = Config::DEFAULT_BPM;
    set_measure_length(16);

    struct timespec system_time;
    clock_gettime(CLOCK_REALTIME, &system_time);
    m_last_time = (system_time.tv_sec * 1000000) + (system_time.tv_nsec / 1000);
}

Engine::~Engine()
{

    for (std::list <Loop>::iterator i = m_loops.begin(); i != m_loops.end(); i++) {
        if ((*i).m_playing) {
            (*i).stop_playing();
        }
    }

    // flush midi output
    snd_seq_drain_output(m_alsa_seq);
    snd_seq_sync_output_queue(m_alsa_seq);

    m_alsa_seq = NULL;
    lo_server_thread_free(m_osc_server);
}

void
Engine::process()
{

    // time
    struct timespec system_time;
    clock_gettime(CLOCK_REALTIME, &system_time);
    long long now_time = (system_time.tv_sec * 1000000) + (system_time.tv_nsec / 1000);
    long long delta_time = now_time - m_last_time;

    // delta time to ticks
    double tick_duration = 1000000 * 60. / m_bpm / Config::PPQN;
    int ticks = (int)(delta_time / tick_duration);
    m_tick += ticks;

    // increment time
    m_last_time += ticks * tick_duration;

    if (ticks > 0) {
        // process loops
        for (std::list <Loop>::iterator i = m_loops.begin(); i != m_loops.end(); i++) {
            (*i).process();
        }
    }

    // receive midi events
    while (poll( m_poll_descriptors, m_num_poll_descriptors, 0) > 0) {
        snd_seq_event_t *ev;
        snd_seq_event_input(m_alsa_seq, &ev);
        // pass event to loop object
        for (std::list <Loop>::iterator i = m_loops.begin(); i != m_loops.end(); i++) {
            if ((*i).m_recording && (*i).m_alsa_port == ev->dest.port) {
                (*i).record_event(*ev);
            }
        }
    }

    // flush midi output
    snd_seq_drain_output(m_alsa_seq);
    snd_seq_sync_output_queue(m_alsa_seq);

}


void
Engine::midi_init()
{

    int ret = snd_seq_open(&m_alsa_seq, "default",  SND_SEQ_OPEN_DUPLEX, 0);

    if (ret < 0) {
    	printf( "snd_seq_open() error\n");
    	exit(1);
    }

    snd_seq_set_client_name(m_alsa_seq, Config::CLIENT_NAME);


    for (int i = 0; i < m_n_loops; i++) {
        std::string name = "Loop " + std::to_string(i);
        int port = snd_seq_create_simple_port(m_alsa_seq,
                name.c_str(),
                SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION
            );
        Loop loop(this, i, m_alsa_seq, port);
        m_loops.push_back(loop);

    }

    m_num_poll_descriptors = snd_seq_poll_descriptors_count(m_alsa_seq, POLLIN);
    m_poll_descriptors = new pollfd[m_num_poll_descriptors];
    snd_seq_poll_descriptors(m_alsa_seq, m_poll_descriptors, m_num_poll_descriptors, POLLIN);

}

void osc_error(int num, const char *m, const char *path)
{
    fprintf(stderr, "liblo server error %d in path %s: %s\n", num, path, m);
}

void
Engine::osc_init()
{

    m_osc_proto = std::string(m_osc_port).find(std::string("osc.unix")) != std::string::npos ? LO_UNIX : LO_DEFAULT;

    if (m_osc_proto == LO_UNIX) {
        m_osc_server = lo_server_thread_new_from_url(m_osc_port, osc_error);
    } else {
        m_osc_server = lo_server_thread_new(m_osc_port, osc_error);
    }

    if (!m_osc_server) exit(1);


    for (int i = 0; i < m_n_loops; i++) {
        std::string path = "/ml/" + std::to_string(i) + "/hit";
        lo_server_thread_add_method(m_osc_server, path.c_str(), "s", Engine::osc_hit_handler, this);

    }
    lo_server_thread_add_method(m_osc_server, "/set", "sf", Engine::osc_ctrl_handler, this);

    lo_server_thread_start(m_osc_server);

}

int
Engine::osc_hit_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data, void *user_data)
{
    Engine *self = (Engine *)user_data;

    int i;
    sscanf (path, "/ml/%i/hit", &i);

    std::list<Loop>::iterator it = self->m_loops.begin();
    std::advance(it, i);


    if (!strcmp(&argv[0]->s, "record")) {
        if ((*it).m_recording) (*it).queue_stop_recording();
        else (*it).queue_start_recording();
    } else if (!strcmp(&argv[0]->s, "mute_off")) {
        (*it).start_playing();
    } else if (!strcmp(&argv[0]->s, "mute_on")) {
        (*it).stop_playing();
    } else if (!strcmp(&argv[0]->s, "clear")) {
        (*it).clear();
    }

    return 0;
}

int
Engine::osc_ctrl_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data, void *user_data)
{
    Engine *self = (Engine *)user_data;

    if (!strcmp(&argv[0]->s, "tempo")) {
        self->set_bpm(argv[1]->f);
    } else if (!strcmp(&argv[0]->s, "eighth_per_cycle")) {
        self->set_measure_length(argv[1]->f);
    }

    return 0;
}


void
Engine::set_measure_length(double eights)
{
    m_length = Config::PPQN * eights / 2;
    for (std::list <Loop>::iterator i = m_loops.begin(); i != m_loops.end(); i++) {
        (*i).m_length = m_length;
    }
}

void
Engine::set_bpm(double bpm)
{
    if (bpm > Config::MAX_BPM) {
        m_bpm = Config::MAX_BPM;
    } else if (bpm < Config::MIN_BPM) {
        m_bpm = Config::MIN_BPM;
    } else {
        m_bpm = bpm;
    }
}
