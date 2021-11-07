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
#ifndef LOOP192_LOOP
#define LOOP192_LOOP

#include <list>
#include <stack>

#include <mutex>

#include <alsa/asoundlib.h>
#include <alsa/seq_midi_event.h>

#include "event.h"
#include "engine.h"

class Engine;

struct Note
{
    int x1; int x2; int y;
};

class Loop
{
    public:

        Loop(Engine * engine, int id, snd_seq_t * seq, int port);
        ~Loop();

        Engine * m_engine;
        int m_id;

        std::mutex  *m_mutex;
        void lock(){m_mutex->lock();};
        void unlock(){m_mutex->unlock();};

        snd_seq_t * m_alsa_seq;
        int m_alsa_port;

        std::list <Event> m_events;
        std::list <Note> m_notes;
        std::list <Note> m_notes_draw;
        std::stack <std::list <Event>> m_events_undo;
        std::stack <std::list <Event>> m_events_redo;
        bool m_has_undo;
        bool m_has_redo;


        long m_tick;
        long m_length;
        long m_starttick;
        long m_lasttick;

        bool m_playing;
        bool m_play_starting;

        bool m_mute;

        bool m_recording;
        bool m_record_starting;
        bool m_record_stopping;

        bool m_overdubbing;

        bool m_queue_overdub_start;
        bool m_queue_overdub_stop;
        bool m_queue_undo;
        bool m_queue_redo;
        bool m_queue_clear;
        int  m_dirty;

        void process();
        void record_event(snd_seq_event_t alsa_event);

        void queue_start_recording();
        void queue_stop_recording();
        void queue_start_playing();
        void start_recording();
        void stop_recording();
        void start_playing();
        void stop_playing();
        void start_overdubbing();
        void stop_overdubbing();
        void set_mute(bool mute);

        void link_notes(bool reset = false);
        void notes_off();
        void clear();

        void push_undo();
        void pop_undo();
        void pop_redo();

        void cache_notes_list();

};

#endif
