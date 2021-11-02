#ifndef MIDILOOPER_EVENT
#define MIDILOOPER_EVENT

#include <alsa/asoundlib.h>
#include <alsa/seq_midi_event.h>

class Event
{
    public:

        Event(snd_seq_event_t alsa_event);
        ~Event();

        long m_timestamp;

        void set_timestamp(long timestamp);
        long get_timestamp();

        void send(snd_seq_t * alsa_seq, int alsa_port);

        snd_seq_event_t m_event;
        Event * m_linked_event;
        bool m_linked;

        bool operator> (Event &event);
        bool operator< (Event &event);

};

#endif
