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

using namespace Gtk;

class Timeline : public DrawingArea {

    public:

        Timeline(Loop * loop);
        ~Timeline();

        Loop            *m_loop;
        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        int              m_dirty;
        int              m_last_marker_pos;
        int              m_next_marker_pos;
        bool             m_queue_draw_background;

        void draw_background();
        void update();

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif
