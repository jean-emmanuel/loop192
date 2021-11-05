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
#ifndef LOOP192_MAINWINDOW
#define LOOP192_MAINWINDOW

#include <list>
#include <gtkmm.h>

#include "../core/engine.h"
#include "loopwidget.h"

using namespace Gtk;

class MainWindow : public Window {

    public:

        MainWindow(Engine * engine);
        ~MainWindow();

        Engine                *m_engine;
        ScrolledWindow         m_scroll;
        VBox                   m_vbox;

        bool timer_callback();

};

#endif
