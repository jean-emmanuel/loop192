#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>

#include "config.hpp"
#include "engine.hpp"

#include "gui_window.hpp"
#include <gtkmm.h>

const char* optstring = "l:p:jnvh";

struct option long_options[] = {
    { "help", 0, 0, 'h' },
    { "loops", 1, 0, 'l' },
    { "osc-port", 1, 0, 'p' },
    { "jack-transport", 0, 0, 'j' },
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
    fprintf(stderr, "  -l <int> , --loops=<int>         number of midi loops (default: %i)\n", DEFAULT_N_LOOPS);
    fprintf(stderr, "  -p <str> , --osc-port=<str>      udp in port number or unix socket path for OSC server (default: %s)\n", DEFAULT_OSC_PORT);
    fprintf(stderr, "  -j , --jack-transport            follow jack transport\n");
    fprintf(stderr, "  -n , --no-gui                    headless mode\n");
    fprintf(stderr, "  -h , --help                      this usage output\n");
    fprintf(stderr, "  -v , --version                   show version only\n");
}

struct OptionInfo
{
    OptionInfo() :
        n_loops(DEFAULT_N_LOOPS),
        port(DEFAULT_OSC_PORT),
        jack_transport(0),
        no_gui(0),
        show_usage(0),
        show_version(0) {}

    int n_loops;

    const char* port;
    const char* target;
    const char* feedback;

    int jack_transport;
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
        case 'p':
            option_info.port = optarg;
            break;
        case 'j':
            option_info.jack_transport++;
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
        fprintf(stdout, " version %s\n", "0.0.0");
        exit(0);
    }
}

Glib::RefPtr<Gtk::Application> application;

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

    OptionInfo option_info;

    parse_options (argc, argv, option_info);

    Engine * engine = new Engine(option_info.n_loops, option_info.port, option_info.jack_transport);

    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGINT, &sighandler);


    if (option_info.no_gui) {
        main_loop((void *)engine);
    } else {
        pthread_create(&thread, NULL, main_loop, engine);
        thread_launched = true;
        application = Gtk::Application::create();
        MainWindow window(engine);
        application->run(window);
        if (thread_launched) {
            run = false;
            pthread_join(thread, NULL);
        }
    }

    delete engine;

    return 0;

}
