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
#ifndef LOOP192_TIMELINEMARKER
#define LOOP192_TIMELINEMARKER

#include <gtkmm.h>

#include "../core/loop.h"

using namespace Gtk;

class TimelineMarker : public DrawingArea {

    public:

        TimelineMarker(Loop * loop);
        ~TimelineMarker();

        Loop            *m_loop;

        int              m_tick;
        int              m_x;
        int              m_h;


    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif
