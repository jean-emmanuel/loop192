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

extern bool global_release_controls_any;
extern int global_release_controls[128];

Loop::Loop(Engine * engine, int id, snd_seq_t * seq, int port)
{
    m_engine = engine;
    m_id = id;
    m_alsa_seq = seq;
    m_alsa_port = port;
    m_mutex = new std::recursive_mutex();

    m_recording = false;
    m_overdubbing = false;
    m_mute = false;

    m_record_starting = false;
    m_record_stopping = false;

    m_lasttick = -1;
    m_starttick = 0;
    m_length = engine->m_length;

    m_dirty = 0;
    m_has_undo = false;
    m_has_redo = false;

    for (int i = 0; i < 128; i++) {
        m_released_controls[i] = 0;
    }
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
                m_tick = m_tick - m_length;
            } else {
                // expand loop length (+1 measure)
                m_length += m_engine->m_length;
                set_dirty();
            }

        }

    } else {

        m_tick = (global_tick - m_starttick) % m_length;

        if (m_record_starting && m_tick < m_lasttick) {
            start_recording();
        }

    }

    // output events
    if (!m_recording && !m_mute && m_events.size() > 0) {
        lock();
        if (m_tick < m_lasttick) {
            // in case we missed some events at the end of the loop
            for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
                if ((*i).get_timestamp() > m_lasttick) {
                    (*i).send(m_alsa_seq, m_alsa_port);
                }
            }
            m_lasttick = -1;
        }

        for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
            if ((*i).get_timestamp() > m_lasttick && (*i).get_timestamp() <= m_tick) {
                (*i).send(m_alsa_seq, m_alsa_port);
            } else if ((*i).get_timestamp() > m_tick) {
                break;
            }
        }
        unlock();

    }

    m_lasttick = m_tick;
}

void
Loop::queue_start_recording()
{
    lock();
    if (!m_record_starting) {
        printf("Loop %i queued start recording\n", m_id);
        m_record_starting = true;
        m_record_stopping = false;
        m_has_undo = true;
    }
    unlock();
}

void
Loop::queue_stop_recording()
{
    lock();
    if (!m_record_stopping) {
        printf("Loop %i queued stop recording\n", m_id);
        m_record_starting = false;
        m_record_stopping = true;
    }
    unlock();
}

void
Loop::start_recording()
{
    lock();
    stop_overdubbing();
    m_record_starting = false;
    if (!m_recording) {
        printf("Loop %i started recording\n", m_id);
        clear();
        m_recording = true;
        m_starttick = m_engine->m_tick;
        push_undo();
    }
    unlock();
}

void
Loop::stop_recording()
{
    lock();
    m_record_stopping = false;
    if (m_recording) {
        printf("Loop %i stopped recording\n", m_id);
        m_recording = false;
        link_notes(true);
    }
    unlock();
}

void
Loop::start_overdubbing()
{
    lock();
    stop_recording();
    if (!m_overdubbing) {
        printf("Loop %i started overdubbing\n", m_id);
        m_overdubbing = true;
        push_undo();
    }
    unlock();
}

void
Loop::stop_overdubbing()
{
    lock();
    if (m_overdubbing) {
        printf("Loop %i stopped overdubbing\n", m_id);
        m_overdubbing = false;
        link_notes(true);
    }
    unlock();
}

void
Loop::set_mute(bool mute)
{
    lock();
    m_mute = mute;
    if (m_mute) notes_off();
    unlock();
}

void
Loop::link_notes(bool reset /*=false*/)
{
    lock();

    if (reset) {
        // unlink
        for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
            (*i).m_linked = false;
            (*i).m_linked_event = NULL;
        }
        m_notes.clear();
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

                    // simple note representation for UI
                    Note n;
                    n.x1 = (*on).get_timestamp();
                    n.x2 = (*off).get_timestamp();
                    n.y = (*on).m_event.data.note.note;
                    m_notes.push_back(n);
                    break;
                }
                off++;
                if (off == m_events.end()) off = m_events.begin();
            }
        }
    }

    if (reset) {
        // insert missing note offs (at last tick)
        for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
            if ((*i).m_event.type == SND_SEQ_EVENT_NOTEON && !(*i).m_linked) {
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

                // simple note representation for UI
                Note n;
                n.x1 = (*i).get_timestamp();
                n.x2 = event->get_timestamp();
                n.y = (*i).m_event.data.note.note;
                m_notes.push_back(n);
            }
        }

        // insert missing note ons (at first tick)
        for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
            if ((*i).m_event.type == SND_SEQ_EVENT_NOTEOFF && !(*i).m_linked) {
                printf("missing on addded\n" );
                snd_seq_event_t noteon = (*i).m_event;
                noteon.type = SND_SEQ_EVENT_NOTEON;
                noteon.data.note.velocity = 100;
                Event * event = new Event(noteon);
                event->set_timestamp(0);
                m_events.push_back(*event);
                m_events.sort();
                (*i).m_linked_event = event;
                event->m_linked_event = &(*i);
                event->m_linked = true;
                (*i).m_linked = true;

                // simple note representation for UI
                Note n;
                n.x1 = (*i).get_timestamp();
                n.x2 = event->get_timestamp();
                n.y = (*i).m_event.data.note.note;
                m_notes.push_back(n);
            }
        }
    }

    set_dirty();

    unlock();

}

void
Loop::notes_off()
{
    lock();

    // play noteoffs, reset pitchbend
    // and user defined controls

    bool released_pitch = false;
    for (int i = 0; i < 128; i++) {
        m_released_controls[i] = 0;
    }

    for (std::list <Event>::iterator i = m_events.begin(); i != m_events.end(); i++) {
        if ((*i).m_event.type == SND_SEQ_EVENT_NOTEON && (*i).m_note_active && (*i).m_linked) {
            (*i).m_linked_event->send(m_alsa_seq, m_alsa_port);
        }
        if (!released_pitch && (*i).m_event.type == SND_SEQ_EVENT_PITCHBEND) {
            snd_seq_event_t pitchoff = (*i).m_event;
            pitchoff.data.control.value = 0;
            Event * event = new Event(pitchoff);
            event->send(m_alsa_seq, m_alsa_port);
            released_pitch = true;
        }
        if ((*i).m_event.type == SND_SEQ_EVENT_CONTROLLER && global_release_controls_any) {
            int cc = (*i).m_event.data.control.param;
            if (global_release_controls[cc] && !m_released_controls[cc]) {
                snd_seq_event_t controloff = (*i).m_event;
                controloff.data.control.value = 0;
                Event * event = new Event(controloff);
                event->send(m_alsa_seq, m_alsa_port);
                m_released_controls[cc] = 1;
            }
        }
    }
    unlock();
}

void
Loop::record_event(snd_seq_event_t alsa_event)
{
    lock();
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
    if (alsa_event.type == SND_SEQ_EVENT_NOTEOFF) {
        link_notes();
    }
    unlock();
}

void
Loop::clear()
{
    lock();
    notes_off();
    m_events.clear();
    m_notes.clear();
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
    set_dirty();
    m_has_undo = false;
    m_has_redo = false;

    unlock();
}

void
Loop::push_undo()
{
    lock();
    if (
        (m_events_undo.size() > 0 && m_events.size() != m_events_undo.top().size()) ||
        m_events_undo.size() == 0
    ) {
        m_events_undo.push(m_events);
        while (!m_events_redo.empty()) {
            m_events_redo.pop();
        }
        set_dirty();
        m_has_redo = false;
        m_has_undo = true;
    }
    unlock();
}

void
Loop::pop_undo()
{
    lock();

    if (m_record_starting)
    {
        m_record_starting = false;
    }
    else if (m_events_undo.size() > 0)
    {
        notes_off();
        m_events_redo.push(m_events);
        m_events = m_events_undo.top();
        m_events_undo.pop();
        m_has_redo = true;
        m_record_stopping = false;
        m_recording = false;
        link_notes(true);
    }
    if (m_events_undo.size() == 0) m_has_undo = false;
    unlock();
}

void
Loop::pop_redo()
{
    lock();
    if (m_events_redo.size() > 0)
    {
        m_events_undo.push(m_events);
        m_events = m_events_redo.top();
        m_events_redo.pop();
        m_has_undo = true;
        link_notes(true);
    }
    if (m_events_redo.size() == 0) m_has_redo = false;
    unlock();
}

void
Loop::cache_notes_list()
{
    lock();
    m_notes_draw = m_notes;
    unlock();
}

void
Loop::set_dirty()
{
    lock();
    m_dirty = true;
    unlock();
}

bool
Loop::is_dirty()
{
    lock();
    bool dirty = m_dirty;
    m_dirty = false;
    unlock();
    return dirty;
}
