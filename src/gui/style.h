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
#ifndef LOOP192_STYLE
#define LOOP192_STYLE

#include <gtkmm.h>


const Gdk::RGBA c_color_background = Gdk::RGBA("#21252b");
const Gdk::RGBA c_color_foreground = Gdk::RGBA("#42454A");
const Gdk::RGBA c_color_raised = Gdk::RGBA("#55575c");
const Gdk::RGBA c_color_text = Gdk::RGBA("#cccccc");
const Gdk::RGBA c_color_text_hilight = Gdk::RGBA("#ffffff");

const Gdk::RGBA c_color_primary = Gdk::RGBA("rgb(117, 170, 229)");
const Gdk::RGBA c_color_secondary = Gdk::RGBA("rgb(229, 170, 117)");

// Main window
const std::string c_mainwindow_css = "\
@define-color color_bg " + c_color_background.to_string() + ";\
@define-color color_fg " + c_color_foreground.to_string() + ";\
@define-color color_raised " + c_color_raised.to_string() + ";\
@define-color color_text " + c_color_text.to_string() + ";\
@define-color color_text_hilight " + c_color_text_hilight.to_string() + ";\
@define-color color_primary " + c_color_primary.to_string() + ";\
@define-color color_secondary " + c_color_secondary.to_string() + ";\
" + R"CSS(

/* reset */
*:not(decoration), window * {
    box-shadow: none;
    border: 0;
    border-radius: 0;
    text-shadow: none;
    color: inherit;
}

/* window */

window {
    background: @color_bg;
    color: @color_text
}


/* scrollbars */

scrollbar {
    background: @color_bg;
    min-width: 12px;
    min-height: 12px;
}

scrollbar slider {
    background: @color_raised;
    min-width: 10px;
    min-height: 10px;
}

scrollbar slider:active {
    background: @color_primary;
}

overshoot, undershoot {
    background: none;
}

/* widgets */

button {
    background: @color_raised;
    box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25);
    min-width: 20px;
    padding: 2px 10px;
}

button:hover {
    opacity: 0.8
}

button:active {
    opacity: 0.6
}

.loopwidget {
    background: @color_fg;
}

)CSS";

#endif
