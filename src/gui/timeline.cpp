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

#include "timeline.h"
#include "style.h"

Timeline::Timeline(Loop * loop) :
    m_loop(loop),
    m_dirty(0),
    m_events(loop),
    m_marker(loop)
{
    add_overlay(m_marker);
    add_overlay(m_events);
}

Timeline::~Timeline()
{

}

void
Timeline::update()
{
    if (m_dirty != m_loop->m_dirty) {
        m_events.queue_draw();
        m_dirty = m_loop->m_dirty;
    }
    if (m_marker.m_tick != m_loop->m_lasttick) {
        // TODO: better perf; handle high speed; 
        m_marker.queue_draw_area(m_marker.m_x - 50, 0, 100, m_marker.m_h);
        if (m_loop->m_lasttick < m_marker.m_tick) {
            m_marker.queue_draw_area(0, 0, 50, m_marker.m_h);
        }
    }
}
