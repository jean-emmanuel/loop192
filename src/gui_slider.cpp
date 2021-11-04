#include "config.hpp"

#include "gui_slider.hpp"
#include "gui_style.h"

LoopSlider::LoopSlider(Loop * loop)
{
    m_loop = loop;

    m_dirty = 0;

    Gtk::Allocation allocation = get_allocation();
    m_surface = Cairo::ImageSurface::create(
        Cairo::Format::FORMAT_ARGB32,
        allocation.get_width(),
        allocation.get_height()
    );

}

LoopSlider::~LoopSlider()
{

}

void
LoopSlider::draw_background()
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
    cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 1.0);

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
                cr->move_to(x1, y);
                cr->line_to(x2, y);
            } else {
                cr->move_to(x1, y);
                cr->line_to(width - 2, y);
                cr->move_to(2, y);
                cr->line_to(x2, y);
            }
        }
    }

    cr->stroke();

    m_dirty = m_loop->m_dirty;

}


bool
LoopSlider::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();
    bool newsurface = false;

    // resize handler
    if (width != m_surface->get_width() || height != m_surface->get_height()){
        m_surface = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            allocation.get_width(),
            allocation.get_height()
        );
        draw_background();
        newsurface = true;
    }

    if (!newsurface && m_dirty != m_loop->m_dirty) // TODO && loop changed
    {
        draw_background();
    }

    // draw background
    cr->set_source(m_surface, 0.0, 0.0);
    cr->paint();

    // draw marker
    int x = 2 + (width - 4) * m_loop->m_lasttick / m_loop->m_length;
    // if (!m_loop->m_playing && !m_loop->m_recording) x = 0;
    auto color = c_color_primary;
    cr->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());
    cr->set_line_width(1.0);
    cr->move_to(x - 0.5, 0);
    cr->line_to(x - 0.5, height);
    cr->stroke();

    return true;
}
