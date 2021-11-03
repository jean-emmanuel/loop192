#ifndef MIDILOOPER_ENGINE
#define MIDILOOPER_ENGINE

#include <list>
#include <lo/lo.h>

#include <alsa/asoundlib.h>
#include <alsa/seq_midi_event.h>

#include <jack/jack.h>
#include <jack/transport.h>

#include "loop.hpp"

class Loop;

class Engine
{
    public:

        Engine(int n_loops, const char* osc_in_port, bool jack_transport);
        ~Engine();

        void process();

        void midi_init();
        void osc_init();
        void jack_init();

        void set_measure_length(double eights);
        void set_bpm(double bpm);

        void start();
        void stop();

        void jack_start();
        void jack_stop();

        const char * m_osc_port;
        lo_server m_osc_server;
        int m_osc_proto;

        snd_seq_t * m_alsa_seq;
        int m_num_poll_descriptors;
        struct pollfd *m_poll_descriptors;


        long long m_last_time;
        long m_tick;
        bool m_playing;
        double m_bpm;
        int m_length;
        int m_n_loops;
        std::list <Loop> m_loops;

        jack_client_t *m_jack_client;
        double m_jack_bpm;
        bool m_jack_running;

        static int osc_hit_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data, void *user_data);
        static int osc_ctrl_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data, void *user_data);
        static int osc_cmd_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data, void *user_data);

        static int jack_process_callback(jack_nframes_t nframes, void* arg);
        static void jack_shutdown(void *arg);

};

#endif
