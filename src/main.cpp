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
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <clocale>

#include "config.h"
#include "core/engine.h"

#include "gui/mainwindow.h"
#include <gtkmm.h>

#include "lib/nsm.h"

bool global_release_controls_any = false;
int global_release_controls[128]{0};

const char* optstring = "l:p:r:jtnvh";

struct option long_options[] = {
    { "help", 0, 0, 'h' },
    { "loops", 1, 0, 'l' },
    { "osc-port", 1, 0, 'p' },
    { "release-controls", 1, 0, 'r' },
    { "jack-transport", 0, 0, 'j' },
    { "tcp", 0, 0, 't' },
    { "no-gui", 0, 0, 'n' },
    { "version", 0, 0, 'v' },
    { 0, 0, 0, 0 }
};

static void usage(char *argv0)
{
    fprintf(stderr, BINARY_NAME);
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Usage: %s [options...]\n", argv0);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -l <int> , --loops=<int>                             number of midi loops (default: %i)\n", DEFAULT_N_LOOPS);
    fprintf(stderr, "  -p <str> , --osc-port=<str>                          udp in port number or unix socket path for OSC server\n");
    fprintf(stderr, "  -r [<int> ...] , --release-controls=[<int> ...]      list of control numbers separated by spaces that should be reset to 0 when muting a loop or stopping transport\n");
    fprintf(stderr, "  -j , --jack-transport                                follow jack transport\n");
    fprintf(stderr, "  -t , --tcp                                           use tcp protocol instead of udp\n");
    fprintf(stderr, "  -n , --no-gui                                        headless mode\n");
    fprintf(stderr, "  -h , --help                                          this usage output\n");
    fprintf(stderr, "  -v , --version                                       show version only\n");
}

struct OptionInfo
{
    OptionInfo() :
        n_loops(DEFAULT_N_LOOPS),
        port(0),
        jack_transport(0),
        osc_tcp(0),
        no_gui(0),
        show_usage(0),
        show_version(0) {}

    int n_loops;

    const char* port;
    const char* target;
    const char* feedback;

    int jack_transport;
    int osc_tcp;
    int no_gui;

    int show_usage;
    int show_version;
    int stress_test;
};

static void parse_options (int argc, char **argv, OptionInfo & option_info)
{
    int longopt_index = 0;
    int c;

    while ((c = getopt_long (argc, argv, optstring, long_options, &longopt_index)) >= 0) {
        if (c >= 255) {
            break;
        }

        switch (c) {
        case 1:
            /* getopt signals end of '-' options */
            break;
        case 'h':
            option_info.show_usage++;
            break;
        case 'v':
            option_info.show_version++;
            break;
        case 'l':
            option_info.n_loops = atoi(optarg);
            break;
        case 'p':
            option_info.port = optarg;
            break;
        case 'r':
        {
            int c = atoi(optarg);
            if (c >= 0 && c < 128) {
                global_release_controls[c] = 1;
                global_release_controls_any = true;
            } else {
                fprintf(stderr, "Control numbers must be between 0 and 127.");
                exit(1);
            }
            break;
        }
        case 'j':
            option_info.jack_transport++;
            break;
        case 't':
            option_info.osc_tcp++;
            break;
        case 'n':
            option_info.no_gui++;
            break;
        default:
            // fprintf (stderr, "argument error: %d\n", c);
            option_info.show_usage++;
            break;
        }

        if (option_info.show_usage > 0) {
            break;
        }
    }

    if (option_info.show_usage) {
        usage(argv[0]);
        exit(1);
    }

    if (option_info.show_version) {
        fprintf(stdout, BINARY_NAME);
        fprintf(stdout, " version %s\n", "0.1.0");
        exit(0);
    }
}

std::string global_client_name = CLIENT_NAME;

// Gtk application
Glib::RefPtr<Gtk::Application> application;

// NSM
nsm_client_t *nsm = 0;
bool nsm_gui = false;
bool global_nsm_gui = false;
bool global_nsm_optional_gui_support = true;
bool nsm_wait = true;
int
nsm_save_cb(char **,  void *userdata)
{
    // nothing to save
    return ERR_OK;
}
void
nsm_hide_cb(void *userdata)
{
    global_nsm_gui = false;
}
void
nsm_show_cb(void *userdata)
{
    global_nsm_gui = true;
}
int
nsm_open_cb(const char *name, const char *display_name, const char *client_id, char **out_msg, void *userdata)
{
    nsm_wait = false;
    global_client_name = client_id;
    nsm_set_save_callback(nsm, nsm_save_cb, 0);
    // NSM API 1.1.0: check if server supports optional-gui
    global_nsm_optional_gui_support = strstr(nsm_get_session_manager_features(nsm), "optional-gui");
    if (global_nsm_optional_gui_support) {
        // make sure nsm server doesn't override cached visibility state
        nsm_send_is_shown(nsm);
        // register optional gui callbacks
        nsm_set_show_callback(nsm, nsm_show_cb, 0);
        nsm_set_hide_callback(nsm, nsm_hide_cb, 0);
    }
    return ERR_OK;
}

// Engine loop
bool run = true;
pthread_t thread;
bool thread_launched;
void* main_loop(void *arg)
{
    Engine * engine = (Engine *)arg;

    struct timespec ts = {
        .tv_sec = 0,
        .tv_nsec = 1000000
    };

    while (run) {
        engine->process();
        nanosleep(&ts, NULL);
    }

    return 0;
}

void sighandler(int sig)
{
    run = false;
    if (thread_launched) {
        pthread_join(thread, NULL);
        thread_launched = false;
        application->quit();
    }
}

int main(int argc, char* argv[])
{

    // ensure dot as decimal separator in json
    std::locale::global(std::locale("C"));

    OptionInfo option_info;

    parse_options (argc, argv, option_info);

    // NSM
    const char *nsm_url = getenv( "NSM_URL" );
    if (!option_info.no_gui && nsm_url) {
        nsm = nsm_new();
        nsm_set_open_callback(nsm, nsm_open_cb, 0);
        if (nsm_init(nsm, nsm_url) == 0) {
            // Announce includes :dirty:capability because we're always clean
            nsm_send_announce(nsm, BINARY_NAME, ":optional-gui:dirty:", argv[0]);
        }
        int timeout = 0;
        while (nsm_wait) {
            nsm_check_wait(nsm, 500);
            timeout += 1;
            if (timeout > 200) exit(1);
        }
    }

    Engine * engine = new Engine(option_info.n_loops, option_info.port, option_info.osc_tcp, option_info.jack_transport);

    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGINT, &sighandler);

    if (option_info.no_gui) {
        main_loop((void *)engine);
    } else {
        pthread_create(&thread, NULL, main_loop, engine);
        thread_launched = true;
        application = Gtk::Application::create();
        MainWindow window(engine, application, nsm);
        application->run(window);
        if (thread_launched) {
            run = false;
            pthread_join(thread, NULL);
        }
    }

    delete engine;

    if (nsm) {
        nsm_free(nsm);
        nsm = NULL;
    }

    return 0;

}
