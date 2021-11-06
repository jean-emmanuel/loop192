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
#ifndef LOOP192_TIMELINE
#define LOOP192_TIMELINE

#include <gtkmm.h>

#include "../core/loop.h"
#include "timeline_events.h"
#include "timeline_marker.h"

using namespace Gtk;

class Timeline : public Overlay {

    public:

        Timeline(Loop * loop);
        ~Timeline();

        Loop            *m_loop;
        int              m_dirty;
        TimelineEvents   m_events;
        TimelineMarker   m_marker;

        void update();

};

#endif
