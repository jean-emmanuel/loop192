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

Timeline::Timeline(Loop * loop)
{
    m_loop = loop;

    m_last_marker_pos = 0;
    m_next_marker_pos = 0;

    Gtk::Allocation allocation = get_allocation();
    m_surface = Cairo::ImageSurface::create(
        Cairo::Format::FORMAT_ARGB32,
        allocation.get_width(),
        allocation.get_height()
    );

}

Timeline::~Timeline()
{

}

void
Timeline::draw_background()
{
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface);
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    cr->set_operator(Cairo::OPERATOR_CLEAR);
    cr->rectangle(-1, -1, width + 2, height + 2);
    cr->paint_with_alpha(1.0);
    cr->set_operator(Cairo::OPERATOR_OVER);

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
    m_loop->cache_notes_list();
    for (std::list <Note>::iterator i = m_loop->m_notes_draw.begin(); i != m_loop->m_notes_draw.end(); i++) {
        if ((*i).y < min_y) min_y = (*i).y;
        if ((*i).y > max_y) max_y = (*i).y;
    }
    min_y -= 2;
    max_y += 2;

    for (std::list <Note>::iterator i = m_loop->m_notes_draw.begin(); i != m_loop->m_notes_draw.end(); i++) {
        int y = height - height * ((*i).y - min_y) / (max_y - min_y);
        int x1 = 2 + (width - 4) * (*i).x1 / length;
        int x2 = 2 + (width - 4) * (*i).x2 / length;
        if ((*i).x2 >= (*i).x1) {
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

    cr->stroke();

}


bool
Timeline::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // resize handler
    if (width != m_surface->get_width() || height != m_surface->get_height()){
        m_surface = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            allocation.get_width(),
            allocation.get_height()
        );
        m_queue_draw_background = true;
    }

    if (m_queue_draw_background)
    {
        draw_background();
        m_queue_draw_background = false;
    }

    // draw background
    cr->set_source(m_surface, 0.0, 0.0);
    cr->paint();

    // draw marker
    if (m_loop->m_mute) {
        auto color = c_color_text;
        cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 0.6);
    } else {
        auto color = c_color_primary;
        cr->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());
    }
    cr->set_line_width(1.0);
    cr->move_to(m_next_marker_pos - 0.5, 0);
    cr->line_to(m_next_marker_pos - 0.5, height);
    cr->stroke();

    m_last_marker_pos = m_next_marker_pos;

    return true;
}

void
Timeline::update()
{
    Gtk::Allocation allocation = get_allocation();
    const int height = allocation.get_height();
    const int width = allocation.get_width();
    m_next_marker_pos = 2 + (width - 4) * m_loop->m_lasttick / m_loop->m_length;

    if (m_loop->is_dirty()) {
        m_queue_draw_background = true;
        queue_draw();
    } else {
        if (m_next_marker_pos > m_last_marker_pos) {
            queue_draw_area(m_last_marker_pos - 1, 0, m_next_marker_pos - m_last_marker_pos + 1, height);
        } else {
            queue_draw_area(m_last_marker_pos - 1, 0, 1, height);
            queue_draw_area(m_next_marker_pos - 1, 0, 1, height);
        }
    }
}
