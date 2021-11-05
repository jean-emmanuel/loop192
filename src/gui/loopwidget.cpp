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

    m_undo.set_label("Undo");
    m_redo.set_label("Redo");
    m_record.set_label("Record");
    m_overdub.set_label("Overdub");
    m_mute.set_label("Mute");

}

LoopWidget::~LoopWidget()
{

}
