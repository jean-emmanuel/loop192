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
#ifndef LOOP192_EVENT
#define LOOP192_EVENT

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
        bool m_note_active;

        bool operator> (Event &event);
        bool operator< (Event &event);

};

#endif
