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

#include "timeline_events.h"
#include "style.h"

TimelineEvents::TimelineEvents(Loop * loop)
{
    m_loop = loop;
}

TimelineEvents::~TimelineEvents()
{

}

bool
TimelineEvents::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{

    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();
    cr->set_line_width(1.0);

    auto color = c_color_text;
    for (int i = 0; i < m_loop->m_length; i+= Config::PPQN / 4){
        if (i % m_loop->m_engine->m_length == 0) {
            cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 0.4);
        } else if (i % Config::PPQN == 0) {
            cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 0.2);
        } else if (i % (Config::PPQN / 4) == 0) {
            cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 0.1);
        } else {
            continue;
        }
        int x = i * width / m_loop->m_length;
        cr->move_to(x - 0.5, 0);
        cr->line_to(x - 0.5, height);
        cr->stroke();
    }

    color = c_color_text;
    cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 0.6);
    cr->set_line_width(2.0);

    int length = m_loop->m_length;
    int max_y = 0;
    int min_y = 127;
    for (std::list <Event>::iterator i = m_loop->m_events.begin(); i != m_loop->m_events.end(); i++) {
        if ((*i).m_event.type == SND_SEQ_EVENT_NOTEON && (*i).m_linked) {
            int note = (*i).m_event.data.note.note;
            if (note < min_y) min_y = note;
            if (note > max_y) max_y = note;
        }
    }
    min_y -= 2;
    max_y += 2;
    for (std::list <Event>::iterator i = m_loop->m_events.begin(); i != m_loop->m_events.end(); i++) {
        if ((*i).m_event.type == SND_SEQ_EVENT_NOTEON && (*i).m_linked) {
            int y = height - height * ((*i).m_event.data.note.note - min_y) / (max_y - min_y);
            int x1 = 2 + (width - 4) * (*i).get_timestamp() / length;
            int x2 = 2 + (width - 4) * (*i).m_linked_event->get_timestamp() / length;
            if ((*i).m_linked_event->get_timestamp() >= (*i).get_timestamp()) {
                if (x2 - x1 < 2) x2 = x1 + 2;
                cr->move_to(x1 - 1, y);
                cr->line_to(x2 - 1, y);
            } else {
                cr->move_to(x1 - 1, y);
                cr->line_to(width - 2, y);
                cr->move_to(2, y);
                cr->line_to(x2 - 1, y);
            }
        }
    }

    cr->stroke();

    return true;
}
