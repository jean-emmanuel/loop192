#ifndef MIDILOOPER_CONFIG
#define MIDILOOPER_CONFIG

#define DEFAULT_OSC_PORT "5244"
#define DEFAULT_N_LOOPS 8
#define BINARY_NAME "midilooper"
#define CLIENT_NAME "MidiLooper"

namespace Config
{

    const double DEFAULT_BPM = 120;
    const double MIN_BPM = 0.1;
    const double MAX_BPM = 3000;

    const int PPQN = 192;
    const int DEFAULT_8TH_PER_CYCLE = 8;

}

#endif
