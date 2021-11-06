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
#include "../config.h"

#include "timeline_marker.h"
#include "style.h"

TimelineMarker::TimelineMarker(Loop * loop)
{
    m_loop = loop;
    m_x = 0;
    m_tick = 0;
}

TimelineMarker::~TimelineMarker()
{

}

bool
TimelineMarker::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    m_h = height;
    m_tick = m_loop->m_lasttick;
    m_x = 2 + (width - 4) * m_loop->m_lasttick / m_loop->m_length;
    float alpha = 1.0;
    auto color = c_color_primary;

    if (m_loop->m_mute) {
        color = c_color_text;
        alpha = 0.6;
    }

    cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), alpha);

    cr->set_line_width(1.0);
    cr->move_to(m_x - 0.5, 0);
    cr->line_to(m_x - 0.5, height);
    cr->stroke();

    return true;
}
