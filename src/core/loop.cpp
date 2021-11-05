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
#include "loop.h"

Loop::Loop(Engine * engine, int id, snd_seq_t * seq, int port)
{
    m_engine = engine;
    m_id = id;
    m_alsa_seq = seq;
    m_alsa_port = port;

    m_recording = false;
    m_overdubbing = false;
    m_playing = false;
    m_mute = false;

    m_play_starting = false;
    m_record_starting = false;
    m_record_stopping = false;

    m_lasttick = 0;
    m_starttick = 0;
    m_length = engine->m_length;

    m_dirty = 0;
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
                m_dirty++;
            }

        }

    } else {

        m_tick = (global_tick - m_starttick) % m_length;

        if (m_record_starting && m_tick < m_lasttick) {
            m_record_starting = false;
            start_recording();
        }

    }

    // output events
    if (m_playing && !m_recording && !m_mute) {

        if (m_tick < m_lasttick && !m_play_starting) {
            // in case we missed some events at the end of the loop
            for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
                if ((*i).get_timestamp() > m_lasttick) {
                    (*i).send(m_alsa_seq, m_alsa_port);
                }
            }
            m_lasttick = 0;
        }

        for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
            if ((*i).get_timestamp() > m_lasttick && (*i).get_timestamp() <= m_tick) {
                (*i).send(m_alsa_seq, m_alsa_port);
            } else if ((*i).get_timestamp() > m_tick) {
                break;
            }
        }

    }

    if (m_tick < m_lasttick && m_play_starting) start_playing();

    m_lasttick = m_tick;

}

void
Loop::queue_start_recording()
{
    stop_playing();
    if (!m_record_starting) {
        printf("Loop %i queued start recording\n", m_id);
        m_record_starting = true;
        m_record_stopping = false;
    }
}

void
Loop::queue_stop_recording()
{
    if (!m_record_stopping) {
        printf("Loop %i queued stop recording\n", m_id);
        m_record_starting = false;
        m_record_stopping = true;
    }
}

void
Loop::start_recording()
{
    stop_overdubbing();
    if (!m_recording) {
        printf("Loop %i started recording\n", m_id);
        clear();
        m_recording = true;
        m_starttick = m_engine->m_tick;
        push_undo();
    }
}

void
Loop::stop_recording()
{
    if (m_recording) {
        printf("Loop %i stopped recording\n", m_id);
        m_recording = false;
        link_notes(true);
    }
}

void
Loop::start_overdubbing()
{
    stop_recording();
    if (!m_overdubbing) {
        printf("Loop %i started overdubbing\n", m_id);
        m_overdubbing = true;
        push_undo();
    }
}

void
Loop::stop_overdubbing()
{
    if (m_overdubbing) {
        printf("Loop %i stopped overdubbing\n", m_id);
        m_overdubbing = false;
        link_notes(true);
    }
}


void
Loop::queue_start_playing()
{
    m_play_starting = true;
}


void
Loop::start_playing()
{
    if (!m_playing) {
        printf("Loop %i started playing\n", m_id);
        m_play_starting = false;
        m_playing = true;
        m_starttick = m_engine->m_tick;
        m_lasttick = m_engine->m_tick % m_length;
    }
}

void
Loop::stop_playing()
{
    if (m_playing) {
        printf("Loop %i stopped playing\n", m_id);
        m_playing = false;
        m_play_starting = false;
        notes_off();
    }
}

void
Loop::link_notes(bool reset /*=false*/)
{
    if (reset) {
        // unlink
        for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
            (*i).m_linked = false;
            (*i).m_linked_event = NULL;
        }
    }

    // link notes
    for (std::list <Event>::iterator on = m_events.begin(); on != m_events.end(); on++) {
        if ((*on).m_event.type == SND_SEQ_EVENT_NOTEON && !(*on).m_linked) {
            std::list <Event>::iterator off = on;
            off++;
            while (off != on) {
                if (
                    (*off).m_event.type == SND_SEQ_EVENT_NOTEOFF &&
                    (*off).m_event.data.note.note == (*on).m_event.data.note.note &&
                    (*off).m_event.data.note.channel == (*on).m_event.data.note.channel &&
                    !(*off).m_linked
                ) {
                    (*on).m_linked_event = &(*off);
                    (*off).m_linked_event = &(*on);
                    (*on).m_linked = true;
                    (*off).m_linked = true;
                    break;
                }
                off++;
                if (off == m_events.end()) off = m_events.begin();
            }
        }
    }

    if (reset) {
        // insert missing note offs
        for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
            if ((*i).m_event.type == SND_SEQ_EVENT_NOTEON && (*i).m_linked_event == NULL) {
                snd_seq_event_t noteoff = (*i).m_event;
                noteoff.type = SND_SEQ_EVENT_NOTEOFF;
                Event * event = new Event(noteoff);
                event->set_timestamp(m_length - 1);
                m_events.push_back(*event);
                m_events.sort();
                (*i).m_linked_event = event;
                event->m_linked_event = &(*i);
                event->m_linked = true;
                (*i).m_linked = true;
            }
        }
    }

    m_dirty++;

}

void
Loop::notes_off()
{
    // play noteoffs
    for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
        if ((*i).m_event.type == SND_SEQ_EVENT_NOTEON && (*i).m_note_active) {
            (*i).m_linked_event->send(m_alsa_seq, m_alsa_port);
        }
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
        if (alsa_event.type == SND_SEQ_EVENT_NOTEOFF) link_notes();


        if (alsa_event.type == SND_SEQ_EVENT_NOTEON) {
            printf("REC: NOTE ON %i\n", alsa_event.data.note.note);
        }
        if (alsa_event.type == SND_SEQ_EVENT_NOTEOFF) {
            printf("REC: NOTE OFF %i\n", alsa_event.data.note.note);
        }
        m_dirty++;
    }
}

void
Loop::clear()
{
    notes_off();
    m_events.clear();
    m_length = m_engine->m_length;
    m_record_starting = false;
    m_record_stopping = false;
    m_recording = false;
    m_overdubbing = false;
    while (!m_events_undo.empty()) {
        m_events_undo.pop();
    }
    while (!m_events_redo.empty()) {
        m_events_redo.pop();
    }
    m_dirty++;
}

void
Loop::push_undo()
{
    if (m_events.size() != m_events_undo.top().size()) {
        m_events_undo.push(m_events);
        // link_notes();
        while (!m_events_redo.empty()) {
            m_events_redo.pop();
        }
        m_dirty++;
    }
}

void
Loop::pop_undo()
{
    if (m_events_undo.size() > 0)
    {
        notes_off();
        m_events_redo.push(m_events);
        m_events = m_events_undo.top();
        m_events_undo.pop();
        link_notes(true);
        m_dirty++;
    }
}

void
Loop::pop_redo()
{
    if (m_events_redo.size() > 0)
    {
        m_events_undo.push(m_events);
        m_events = m_events_redo.top();
        m_events_redo.pop();
        link_notes(true);
        m_dirty++;
    }
}
