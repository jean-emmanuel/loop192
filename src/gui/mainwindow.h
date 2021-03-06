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

#include "../lib/nsm.h"

using namespace Gtk;

class MainWindow : public Window {

    public:

        MainWindow(Engine * engine, Glib::RefPtr<Gtk::Application> app, nsm_client_t * nsm);
        ~MainWindow();

        Engine                *m_engine;
        Glib::RefPtr<Gtk::Application> m_app;

        // menu
        MenuBar                m_menu;
        MenuItem               m_menu_file;
        Menu                   m_submenu_file;
        MenuItem               m_menu_file_quit;
        MenuItem               m_menu_transport;
        Menu                   m_submenu_transport;
        AccelLabel             m_menu_transport_start_label;
        MenuItem               m_menu_transport_start;
        AccelLabel             m_menu_transport_stop_label;
        MenuItem               m_menu_transport_stop;

        ScrolledWindow         m_scroll;
        VBox                   m_vbox;
        HBox                   m_toolbar;
        VBox                   m_loops;

        Button                 m_toolbar_panic;
        Button                 m_toolbar_play;
        Button                 m_toolbar_stop;

        Button                 m_toolbar_bpm_label;
        SpinButton             m_toolbar_bpm;
        Glib::RefPtr<Gtk::Adjustment> m_toolbar_bpm_adj;
        Button                 m_toolbar_length_label;
        SpinButton             m_toolbar_length;
        Glib::RefPtr<Gtk::Adjustment> m_toolbar_length_adj;
        Image                  m_toolbar_logo;

        bool                   m_toolbar_play_state;

        // NSM
        nsm_client_t          *m_nsm;
        bool                   m_nsm_visible;
        void nsm_set_client(nsm_client_t *nsm);

        bool timer_callback();
        void clear_focus();

        bool on_delete_event(GdkEventAny *event);

};

#endif
