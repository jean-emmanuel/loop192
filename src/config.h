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
#ifndef LOOP192_CONFIG
#define LOOP192_CONFIG

#define DEFAULT_OSC_PORT "5244"
#define DEFAULT_N_LOOPS 8
#define BINARY_NAME "loop192"
#define CLIENT_NAME "Loop192"

extern bool global_nsm_gui;
extern bool global_nsm_optional_gui_support;

namespace Config
{

    const double DEFAULT_BPM = 120;
    const double MIN_BPM = 0.1;
    const double MAX_BPM = 3000;

    const int PPQN = 192;

    const double DEFAULT_8TH_PER_CYCLE = 8;
    const double MIN_8TH_PER_CYCLE = 1;
    const double MAX_8TH_PER_CYCLE = 200;

}

#endif
