#ifndef MIDILOOPER_GUISLIDER
#define MIDILOOPER_GUISLIDER

#include <gtkmm.h>

#include "loop.hpp"

using namespace Gtk;

class LoopSlider : public DrawingArea {

    public:

        LoopSlider(Loop * loop);
        ~LoopSlider();

        Loop            *m_loop;
        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        void draw_background();

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif
