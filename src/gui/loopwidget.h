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
#ifndef LOOP192_LOOPWIDGET
#define LOOP192_LOOPWIDGET

#include <list>
#include <gtkmm.h>

#include "timeline.h"
#include "../core/loop.h"

using namespace Gtk;

class LoopWidget : public Box {

    public:

        LoopWidget(Loop * loop);
        ~LoopWidget();

        Loop            *m_loop;

        Box              m_vbox1;
        Box              m_vbox2;
        Box              m_vbox3;
        Timeline         m_timeline;

        Label            m_label;
        Button           m_undo;
        Button           m_redo;
        Button           m_record;
        Button           m_overdub;
        Button           m_mute;
        Button           m_clear;

        bool m_undo_state;
        bool m_redo_state;
        bool m_record_state;
        bool m_overdub_state;
        bool m_mute_state;

        void timer_callback();


};

#endif
