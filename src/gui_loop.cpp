#include "gui_loop.hpp"
#include "gui_slider.hpp"
#include "gui_style.h"

LoopWidget::LoopWidget(Loop * loop) : m_progress(loop)
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

    // LoopSlider m_progress(m_loop);
    pack_start(m_progress);
    m_progress.set_hexpand(true);
    m_progress.set_halign(Gtk::ALIGN_FILL);

    m_undo.set_label("Undo");
    m_redo.set_label("Redo");
    m_record.set_label("Record");
    m_overdub.set_label("Overdub");
    m_mute.set_label("Mute");

}

LoopWidget::~LoopWidget()
{

}
