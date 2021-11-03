#include "gui_window.hpp"
#include "gui_loop.hpp"
#include "gui_style.h"

MainWindow::MainWindow(Engine * engine)
{
    m_engine = engine;

    Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(c_mainwindow_css);
    this->get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    add(m_scroll);
    m_scroll.add(m_vbox);

    m_vbox.set_spacing(2);
    for (std::list <Loop>::iterator i = engine->m_loops.begin(); i != engine->m_loops.end(); i++) {
        LoopWidget * w = new LoopWidget(&(*i));
        m_vbox.pack_start(*w, false, false);
    }

    // timer callback (25 fps)
    Glib::signal_timeout().connect(mem_fun(*this, &MainWindow::timer_callback), 20);

    resize(600, 200);
    show_all();
}

MainWindow::~MainWindow()
{

}

bool
MainWindow::timer_callback()
{
    auto children = m_vbox.get_children();
    for (auto i = children.begin(); i != children.end(); i++) {
        LoopWidget *w = (LoopWidget *)(*i);
        w->m_progress.queue_draw();
    }

    return true;
}
