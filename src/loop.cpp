#include "loop.hpp"

Loop::Loop(Engine * engine, int id, snd_seq_t * seq, int port)
{
    m_engine = engine;
    m_id = id;
    m_alsa_seq = seq;
    m_alsa_port = port;

    m_recording = false;
    m_playing = false;

    m_play_starting = false;
    m_record_starting = false;
    m_record_stopping = false;

    m_lasttick = 0;
    m_starttick = 0;
    m_length = 0;
}

Loop::~Loop()
{

}

void
Loop::process()
{
    long global_tick = m_engine->m_tick;

    if (m_recording) {

        m_tick = global_tick - m_starttick;

        // if we reached the end of the loop
        if (m_tick >= m_length) {
            if (m_record_stopping) {
                // stop recording
                stop_recording();
                queue_start_playing();
            } else {
                // expand loop length (+1 measure)
                m_length += m_engine->m_length;
            }

        }

    } else {

        m_tick = global_tick % m_length;

        if (m_record_starting && m_tick < m_lasttick) {
            m_record_starting = false;
            start_recording();
        }

    }

    if (m_tick < m_lasttick && m_play_starting) start_playing();

    // output events
    if (m_playing && !m_recording) {

        if (m_tick < m_lasttick) {
            for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
                if ((*i).get_timestamp() > m_lasttick) {
                    printf("send note\n");
                    (*i).send(m_alsa_seq, m_alsa_port);
                }
            }
            m_lasttick = 0;
        }

        for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
            if ((*i).get_timestamp() > m_lasttick && (*i).get_timestamp() <= m_tick) {
                printf("send note\n");
                (*i).send(m_alsa_seq, m_alsa_port);
            } else if ((*i).get_timestamp() > m_tick) {
                break;
            }
        }

    }

    m_lasttick = m_tick;

}

void
Loop::queue_start_recording()
{
    stop_playing();
    printf("Loop %i queued start recording\n", m_id);
    m_record_starting = true;
    m_record_stopping = false;
}

void
Loop::queue_stop_recording()
{
    printf("Loop %i queued stop recording\n", m_id);
    m_record_starting = false;
    m_record_stopping = true;
}

void
Loop::start_recording()
{
    printf("Loop %i started recording\n", m_id);
    clear();
    m_recording = true;
    m_starttick = m_engine->m_tick;
}

void
Loop::stop_recording()
{
    printf("Loop %i stopped recording\n", m_id);
    m_recording = false;
    link_notes();
}

void
Loop::queue_start_playing()
{
    m_play_starting = true;
}


void
Loop::start_playing()
{
    printf("Loop %i started playing\n", m_id);
    m_play_starting = false;
    m_playing = true;
    m_lasttick = m_engine->m_tick % m_length;
}

void
Loop::stop_playing()
{
    printf("Loop %i stopped playing\n", m_id);
    m_playing = false;
    m_play_starting = false;

    // play noteoffs
    for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
        if (
            (*i).get_timestamp() <= m_lasttick &&
            (*i).m_event.type == SND_SEQ_EVENT_NOTEON &&
            (*i).m_linked_event->get_timestamp() > m_lasttick
        ) {
            (*i).m_linked_event->send(m_alsa_seq, m_alsa_port);
        } else if ((*i).get_timestamp() > m_lasttick) {
            break;
        }
    }
}

void
Loop::link_notes()
{
    // link notes and add missing noteoffs

    for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
        if ((*i).m_event.type == SND_SEQ_EVENT_NOTEON) {
            for (std::list <Event>::iterator j = m_events.begin(); j != m_events.end(); j++) {
                if (
                    (*j).m_event.type == SND_SEQ_EVENT_NOTEOFF &&
                    (*j).m_event.data.note.note == (*i).m_event.data.note.note &&
                    (*j).m_event.data.note.channel == (*i).m_event.data.note.channel &&
                    !(*j).m_marked
                ) {
                    (*j).m_marked = true;
                    (*i).m_linked_event = &(*j);
                    break;
                }
            }
            if ((*i).m_linked_event == NULL) {
                snd_seq_event_t noteoff = (*i).m_event;
                noteoff.type = SND_SEQ_EVENT_NOTEOFF;
                Event * event = new Event(noteoff);
                event->set_timestamp(m_length - 1);
                m_events.push_back(*event);
                m_events.sort();
                (*i).m_linked_event = event;
            }
        }
    }
    for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
        if ((*i).m_marked) (*i).m_marked = false;
    }
}

void
Loop::record_event(snd_seq_event_t alsa_event)
{
    if (
        alsa_event.type == SND_SEQ_EVENT_NOTEON ||
        alsa_event.type == SND_SEQ_EVENT_NOTEOFF ||
        alsa_event.type == SND_SEQ_EVENT_CONTROLLER ||
        alsa_event.type == SND_SEQ_EVENT_PITCHBEND
    ) {
        Event event(alsa_event);
        event.set_timestamp(m_tick);
        m_events.push_back(event);
        m_events.sort();
    }
}

void
Loop::clear()
{
    m_events.clear();
    m_length = m_engine->m_length;
    m_record_starting = false;
    m_record_stopping = false;
    m_recording = false;
}
