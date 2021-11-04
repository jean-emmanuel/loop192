#ifndef MIDILOOPER_LOOP
#define MIDILOOPER_LOOP

#include <list>
#include <stack>

#include <alsa/asoundlib.h>
#include <alsa/seq_midi_event.h>

#include "event.hpp"
#include "engine.hpp"

class Engine;

class Loop
{
    public:

        Loop(Engine * engine, int id, snd_seq_t * seq, int port);
        ~Loop();

        Engine * m_engine;
        int m_id;

        snd_seq_t * m_alsa_seq;
        int m_alsa_port;

        std::list <Event> m_events;
        std::stack <std::list <Event>> m_events_undo;
        std::stack <std::list <Event>> m_events_redo;


        long m_tick;
        long m_length;
        long m_starttick;
        long m_lasttick;

        bool m_playing;
        bool m_play_starting;
        bool m_recording;
        bool m_record_starting;
        bool m_record_stopping;
        bool m_overdubbing;

        int m_dirty;

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

        void link_notes(bool reset = false);
        void notes_off();
        void clear();

        void push_undo();
        void pop_undo();
        void pop_redo();

};

#endif
