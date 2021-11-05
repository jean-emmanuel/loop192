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
#include <string>

#include "../config.h"
#include "engine.h"
#include "event.h"

Engine::Engine(int n_loops, const char* osc_in_port, bool jack_transport)
{
    m_osc_port = osc_in_port;
    m_n_loops = n_loops;

    m_playing = false;
    m_tick = 0;
    m_bpm = 0;
    m_last_time = 0;
    m_jack_running = false;
    m_length = 0;

    set_bpm(Config::DEFAULT_BPM);
    set_measure_length(Config::DEFAULT_8TH_PER_CYCLE);

    midi_init();
    osc_init();

    if (jack_transport) jack_init();
}

Engine::~Engine()
{

    stop();

    // flush midi output
    snd_seq_drain_output(m_alsa_seq);
    snd_seq_sync_output_queue(m_alsa_seq);

    m_alsa_seq = NULL;
    lo_server_free(m_osc_server);
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

    // increment time
    m_last_time += ticks * tick_duration;

    if (m_playing && ticks > 0) {
        m_tick += ticks;
        // process loops
        for (std::list <Loop>::iterator i = m_loops.begin(); i != m_loops.end(); i++) {
            (*i).process();
        }
    }

    // receive midi events
    while (snd_seq_event_input_pending(m_alsa_seq, 1)) {
        snd_seq_event_t *ev;
        snd_seq_event_input(m_alsa_seq, &ev);
        // pass event to loop object
        for (std::list <Loop>::iterator i = m_loops.begin(); i != m_loops.end(); i++) {
            if (((*i).m_recording || (*i).m_overdubbing) && (*i).m_alsa_port == ev->dest.port) {
                (*i).record_event(*ev);
            }
        }
    }

    // receive osc events
    while (lo_server_recv_noblock(m_osc_server, 0)) {}

    // flush midi output
    snd_seq_drain_output(m_alsa_seq);
    snd_seq_sync_output_queue(m_alsa_seq);

}

void
Engine::midi_init()
{

    int ret = snd_seq_open(&m_alsa_seq, "default",  SND_SEQ_OPEN_DUPLEX, 0);

    if (ret < 0) {
    	printf("snd_seq_open() error\n");
    	exit(1);
    }

    snd_seq_set_client_name(m_alsa_seq, CLIENT_NAME);


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

}

void osc_error(int num, const char *m, const char *path)
{
    fprintf(stderr, "liblo server error %d in path %s: %s\n", num, path, m);
}

void
Engine::osc_init()
{

    int proto = std::string(m_osc_port).find(std::string("osc.unix")) != std::string::npos ? LO_UNIX : LO_DEFAULT;

    if (proto == LO_UNIX) {
        m_osc_server = lo_server_new_from_url(m_osc_port, osc_error);
    } else {
        m_osc_server = lo_server_new(m_osc_port, osc_error);
    }

    if (!m_osc_server) exit(1);


    for (int i = 0; i < m_n_loops; i++) {
        std::string path = "/ml/" + std::to_string(i) + "/hit";
        lo_server_add_method(m_osc_server, path.c_str(), "s", Engine::osc_hit_handler, this);

    }
    lo_server_add_method(m_osc_server, "/set", "sf", Engine::osc_ctrl_handler, this);
    lo_server_add_method(m_osc_server, "/play", "", Engine::osc_cmd_handler, this);
    lo_server_add_method(m_osc_server, "/stop", "", Engine::osc_cmd_handler, this);
    lo_server_add_method(m_osc_server, "/status", "", Engine::osc_cmd_handler, this);
    lo_server_add_method(m_osc_server, "/status", "s", Engine::osc_cmd_handler, this);

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
    } else if (!strcmp(&argv[0]->s, "overdub")) {
        if ((*it).m_overdubbing) (*it).stop_overdubbing();
        else (*it).start_overdubbing();
    } else if (!strcmp(&argv[0]->s, "mute_off")) {
        (*it).m_mute = false;
    } else if (!strcmp(&argv[0]->s, "mute_on")) {
        (*it).m_mute = true;
        (*it).notes_off();
    } else if (!strcmp(&argv[0]->s, "clear")) {
        (*it).clear();
    } else if (!strcmp(&argv[0]->s, "undo")) {
        (*it).pop_undo();
    } else if (!strcmp(&argv[0]->s, "redo")) {
        (*it).pop_redo();
    }

    return 0;
}

int
Engine::osc_ctrl_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data, void *user_data)
{
    Engine *self = (Engine *)user_data;

    if (!strcmp(&argv[0]->s, "tempo")) {
        self->set_bpm(argv[1]->f);
    } else if (!strcmp(&argv[0]->s, "eigth_per_cycle")) {
        self->set_measure_length(argv[1]->f);
    }

    return 0;
}

int
Engine::osc_cmd_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data, void *user_data)
{
    Engine *self = (Engine *)user_data;

    if (!strcmp(path, "/play")) {
        if (self->m_jack_running) self->jack_start();
        else self->start();
    } else if (!strcmp(path, "/stop")) {
        if (self->m_jack_running) self->jack_stop();
        else self->stop();
    } else if (!strcmp(path, "/status")) {

        std::string json = "{";
        json += "\"playing\":" + std::to_string(self->m_playing) + ",";
        json += "\"tick\":" + std::to_string(self->m_tick) + ",";
        json += "\"length\":" + std::to_string(self->m_length) + ",";
        json += "\"bpm\":" + std::to_string(self->m_bpm) + ",";
        json += "\"loops\":[";
        for (std::list <Loop>::iterator i = self->m_loops.begin(); i != self->m_loops.end(); i++) {
            json += "{";
            json += "\"id\":" + std::to_string((*i).m_id) + ",";
            json += "\"length\":" + std::to_string((*i).m_length) + ",";
            json += "\"playing\":" + std::to_string((*i).m_playing) + ",";
            json += "\"recording\":" + std::to_string((*i).m_recording) + ",";
            json += "\"record_starting\":" + std::to_string((*i).m_record_starting) + ",";
            json += "\"record_stopping\":" + std::to_string((*i).m_record_stopping) + ",";
            json += "\"overdubbing\":" + std::to_string((*i).m_overdubbing);
            json += "},";
        }
        json = json.substr(0, json.size() - 1);
        json += "]}";

        lo_address address;
        if (argc == 1) {
            address = lo_address_new_from_url(&argv[0]->s);
            if (address == NULL) {
                printf("Invalid osc address: %s\n", &argv[0]->s);
                return 1;
            }
        } else {
            address = lo_address_new_from_url(lo_address_get_url(lo_message_get_source(data)));
            if (address == NULL) {
                printf("Could not find origin osc address\n");
                return 1;
            }
        }

        lo_server from = lo_server_get_protocol(self->m_osc_server) == LO_UNIX ? NULL : self->m_osc_server;
        lo_send_from(address, from, LO_TT_IMMEDIATE, "/status", "s", json.c_str());
        lo_address_free(address);

    }

    return 0;
}

void
Engine::jack_init()
{

    m_jack_client = jack_client_open(CLIENT_NAME, JackNullOption, NULL);

    if (m_jack_client == 0)
    {
        printf("Jack server is not running. Jack transport sync disabled.\n");
        return;
    }

    jack_on_shutdown(m_jack_client, Engine::jack_shutdown, this);
    jack_set_process_callback(m_jack_client, Engine::jack_process_callback, this);

    if (jack_activate(m_jack_client))
    {
        printf("Cannot register as Jack client\n");
        return;
    }

    m_jack_running = true;
}

void
Engine::jack_shutdown(void *user_data)
{
    Engine *self = (Engine *)user_data;
    self->m_jack_running = false;
    printf("Jack shut down. Jack transport sync disabled.\n");
}

int
Engine::jack_process_callback(jack_nframes_t nframes, void* user_data)
{
    Engine *self = (Engine *)user_data;

    jack_position_t pos;
    jack_transport_state_t state = jack_transport_query(self->m_jack_client, &pos);

    if (pos.beats_per_minute > Config::MIN_BPM) {
        self->set_bpm(pos.beats_per_minute);
    }

    if (state == JackTransportRolling) {
        if (!self->m_playing) {
            printf("Jack transport is rolling\n");
            self->start();
        }
    } else if (state == JackTransportStopped || state == JackTransportStarting) {
        if (self->m_playing) {
            printf("Jack transport is stopped\n");
            self->stop();
        }
    }

    return 0;
}


void
Engine::set_measure_length(double eights)
{
    int length = Config::PPQN * eights / 2;
    if (length != m_length) {
        m_length = length;
        for (std::list <Loop>::iterator i = m_loops.begin(); i != m_loops.end(); i++) {
            (*i).clear();
            (*i).m_length = m_length;
        }
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


void
Engine::start()
{
    bool trig = m_playing;

    if (trig) {
        printf("Engine transport restarted\n");
    } else {
        printf("Engine transport started\n");
    }

    m_playing = true;
    m_tick = 0;
    struct timespec system_time;
    clock_gettime(CLOCK_REALTIME, &system_time);
    m_last_time = (system_time.tv_sec * 1000000) + (system_time.tv_nsec / 1000);

    for (std::list <Loop>::iterator i = m_loops.begin(); i != m_loops.end(); i++) {
        if ((*i).m_playing) {
            (*i).stop_playing();
            (*i).queue_start_playing();
        }
        if ((*i).m_recording) (*i).stop_recording();
        if ((*i).m_overdubbing) (*i).stop_overdubbing();
    }

}

void
Engine::jack_start()
{
    if (m_playing) {
        printf("Jack transport restarted\n");
        jack_transport_locate(m_jack_client, 0);
    } else {
        printf("Jack transport started\n");
        jack_transport_start(m_jack_client);
    }
}


void
Engine::jack_stop()
{
    printf("Jack transport stopped\n");
    jack_transport_stop(m_jack_client);
}


void
Engine::stop()
{
    if (m_playing) {
        printf("Engine transport stopped\n");
        m_tick = 0;
        m_playing = false;
        for (std::list <Loop>::iterator i = m_loops.begin(); i != m_loops.end(); i++) {
            (*i).notes_off();
        }
    }
}
