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
#include "event.h"

Event::Event(snd_seq_event_t alsa_event)
{
    m_event = alsa_event;
    m_linked = false;
    m_linked_event = NULL;
    m_note_active = false;
}

Event::~Event()
{

}

void
Event::set_timestamp(long timestamp)
{
    m_timestamp = timestamp;
}

long
Event::get_timestamp()
{
    return m_timestamp;
}

void
Event::send(snd_seq_t * alsa_seq, int alsa_port)
{
    snd_seq_ev_set_direct(&m_event);
    snd_seq_ev_set_source(&m_event, alsa_port);
    snd_seq_ev_set_subs(&m_event);
    snd_seq_event_output(alsa_seq, &m_event);

    if (m_event.type == SND_SEQ_EVENT_NOTEON) m_note_active = true;
    else if (m_event.type == SND_SEQ_EVENT_NOTEOFF && m_linked) {
        m_linked_event->m_note_active = false;
    }
}

bool
Event::operator>(Event &event)
{
    return get_timestamp() > event.get_timestamp();
}

bool
Event::operator<(Event &event)
{
    return get_timestamp() < event.get_timestamp();
}
