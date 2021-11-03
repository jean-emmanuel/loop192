#ifndef MIDILOOPER_GUILOOP
#define MIDILOOPER_GUILOOP

#include <list>
#include <gtkmm.h>

#include "gui_slider.hpp"
#include "loop.hpp"

using namespace Gtk;

class LoopWidget : public Box {

    public:

        LoopWidget(Loop * loop);
        ~LoopWidget();

        Loop            *m_loop;

        Box              m_vbox1;
        Box              m_vbox2;
        Box              m_vbox3;
        LoopSlider       m_progress;
        Glib::RefPtr<Gtk::Adjustment> m_progress_adj;

        Button           m_undo;
        Button           m_redo;
        Button           m_record;
        Button           m_overdub;
        Button           m_mute;


};

#endif
