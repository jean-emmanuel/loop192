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
#include "loopwidget.h"
#include "style.h"

LoopWidget::LoopWidget(Loop * loop) : m_timeline(loop)
{
    m_loop = loop;

    get_style_context()->add_class("loopwidget");

    set_orientation(Gtk::Orientation::ORIENTATION_HORIZONTAL);

    pack_start(m_label, false, false);
    m_label.set_label(std::to_string(m_loop->m_id));
    m_label.set_sensitive(false);
    m_label.get_style_context()->add_class("label");
    m_label.set_size_request(60, 0);

    pack_start(m_vbox1, false, false);
    m_vbox1.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
    m_vbox1.set_homogeneous(true);
    m_vbox1.add(m_undo);
    m_vbox1.add(m_redo);

    pack_start(m_vbox2, false, false);
    m_vbox2.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
    m_vbox2.set_homogeneous(true);
    m_vbox2.add(m_record);
    m_vbox2.add(m_overdub);
    m_vbox2.add(m_mute);

    pack_start(m_timeline);
    m_timeline.set_hexpand(true);
    m_timeline.set_halign(Gtk::ALIGN_FILL);


    pack_start(m_clear,false,false);

    m_undo.set_label("Undo");
    m_redo.set_label("Redo");
    m_record.set_label("Record");
    m_overdub.set_label("Overdub");
    m_mute.set_label("Mute");
    m_clear.set_label("Clear");

    m_record.get_style_context()->add_class("record");
    m_overdub.get_style_context()->add_class("overdub");
    m_mute.get_style_context()->add_class("mute");

    m_undo.set_can_focus(false);
    m_redo.set_can_focus(false);
    m_record.set_can_focus(false);
    m_overdub.set_can_focus(false);
    m_mute.set_can_focus(false);
    m_clear.set_can_focus(false);

    m_record_state = false;
    m_overdub_state = false;
    m_mute_state = false;
    m_undo_state = false;
    m_redo_state = false;
    m_wait_state = false;

    m_undo.set_sensitive(false);
    m_redo.set_sensitive(false);

    m_record.signal_clicked().connect([&]{
        if (m_loop->m_recording) m_loop->queue_stop_recording();
        else m_loop->queue_start_recording();
    });

    m_overdub.signal_clicked().connect([&]{
        if (m_loop->m_overdubbing) m_loop->stop_overdubbing();
        else m_loop->start_overdubbing();
    });

    m_mute.signal_clicked().connect([&]{
        m_loop->set_mute(!m_loop->m_mute);
    });

    m_undo.signal_clicked().connect([&]{
        m_loop->pop_undo();
    });

    m_redo.signal_clicked().connect([&]{
        m_loop->pop_redo();
    });

    m_clear.signal_clicked().connect([&]{
        m_loop->clear();
    });

}

LoopWidget::~LoopWidget()
{

}

void
LoopWidget::timer_callback()
{
    // update timeline
    m_timeline.update();

    if (m_loop->m_recording != m_record_state) {
        m_record_state = m_loop->m_recording;
        if (m_record_state) m_record.get_style_context()->add_class("on");
        else m_record.get_style_context()->remove_class("on");
    }

    if (m_loop->m_overdubbing != m_overdub_state) {
        m_overdub_state = m_loop->m_overdubbing;
        if (m_overdub_state) m_overdub.get_style_context()->add_class("on");
        else m_overdub.get_style_context()->remove_class("on");
    }

    if (m_loop->m_mute != m_mute_state) {
        m_mute_state = m_loop->m_mute;
        if (m_mute_state) m_mute.get_style_context()->add_class("on");
        else m_mute.get_style_context()->remove_class("on");
    }

    if (m_loop->m_has_undo != m_undo_state) {
        m_undo_state = m_loop->m_has_undo;
        m_undo.set_sensitive(m_undo_state);
    }

    if (m_loop->m_has_redo != m_redo_state) {
        m_redo_state = m_loop->m_has_redo;
        m_redo.set_sensitive(m_redo_state);
    }

    bool wait = m_loop->m_record_starting || m_loop->m_record_stopping;
    if (wait != m_wait_state) {
        m_wait_state = wait;
        if (m_wait_state) m_record.get_style_context()->add_class("waiting");
        else m_record.get_style_context()->remove_class("waiting");
    }

}
