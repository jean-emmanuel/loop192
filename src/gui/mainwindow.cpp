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
#include "mainwindow.h"
#include "style.h"

MainWindow::MainWindow(Engine * engine)
{
    m_engine = engine;

    Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(c_mainwindow_css);
    this->get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    add(m_scroll);
    m_scroll.add(m_vbox);

    m_vbox.set_spacing(1);
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
        w->m_timeline.queue_draw();
    }

    return true;
}
