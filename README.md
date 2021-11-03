# midilooper

Minimal MIDI live looper that works like sooperlooper but with MIDI instead of audio.

## OSC
```
LOOP COMMANDS:

/ml/#/hit s:cmdname
    A single hit only, no press-release action
    Where # is the loop index starting from 0.
    Specifying * will affect all loops.

    Where cmdname is one of the following:

    record
    overdub
    undo
    redo
    mute_on
    mute_off
    clear


GLOBAL COMMANDS:

/set  s:param  f:value
    where param is one of:

    tempo
    eighth_per_cycle

/start
    Start transport (reset playhead position to 0)

/stop
    Stop transport

/status
/status s:return_url
    Send status as a JSON string to return_url (osc.udp:// or osc.unix:// address)
    If return_url is omitted, the sender's address will be used.
    If midilooper is bound to a unix socket, the message will be sent from a random udp port, otherwise it will be sent from midilooper's osc port.  

STATUS MESSAGE: /status s:json

    {
      "playing": 1,             // 0 or 1
      "tick": 0,                // playhead position*
      "length": 768,            // cycle length*
      "bpm": 120,               // tempo
      "loops": [
        {
          "id": 0,              // loop number
          "length": 768,        // loop length* (multiple of engine's length)
          "playing": 0,         // 0 or 1
          "recording": 0,       // 0 or 1
          "record_starting": 0, // 0 or 1
          "record_stopping": 0, // 0 or 1
          "overdubbing": 0      // 0 or 1
        },
        ...
      ]
    }

    * 192 ticks per quarter notes
```
