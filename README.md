# midilooper

A minimal MIDI live looper that works like sooperlooper but with MIDI instead of audio.

## OSC
```
LOOPS COMMANDS:

/ml/#/hit s:cmdname
    A single hit only, no press-release action
    Where # is the loop index starting from 0.
    Specifying * will affect all loops.

    Where cmdname is one of the following:

    record
    mute_on
    mute_off
    clear


GLOBAL PARAMETERS:

/set  s:param  f:value
    where param is one of:

    tempo
    eighth_per_cycle

```
