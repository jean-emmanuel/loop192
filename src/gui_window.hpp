#ifndef MIDILOOPER_WINDOW
#define MIDILOOPER_WINDOW

#include <list>
#include <gtkmm.h>

#include "engine.hpp"
#include "gui_loop.hpp"

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
