// This file is part of loop192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef LOOP192_ENGINE
#define LOOP192_ENGINE

#include <list>
#include <lo/lo.h>

#include <alsa/asoundlib.h>
#include <alsa/seq_midi_event.h>

#include <jack/jack.h>
#include <jack/transport.h>

#include "loop.h"

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

        snd_seq_t * m_alsa_seq;

        long long m_last_time;
        long m_tick;
        bool m_playing;
        double m_bpm;
        double get_bpm(){return m_bpm;};
        int m_length;
        int get_length(){return m_length;};
        int m_queue_length;

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
