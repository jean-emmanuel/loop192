loop192(1) -- live MIDI sequencer
=============================

## SYNOPSIS

`loop192` [OPTION...]

## DESCRIPTION

loop192 is a MIDI looper that works like sooperlooper but with MIDI instead of audio.

## OPTIONS

* `-h, --help`:
    Show available options

* `-p, --osc-port` <port>:
    OSC input port (udp port number or unix socket path)

* `-j, --jack-transport`:
    Sync to jack transport

* `-n, --no-gui`:
    Enable headless mode

* `-v, --version`:
    Show version and exit

## USER INTERFACE

TODO

## JACK TRANSPORT

When `--jack-transport` is set, loop192 will

    - follow start / stop commands from other clients
    - send start / stop commands to other clients
    - use the transport master's bpm
    - set its position to 0 whenever the transport stops or restarts
    - **not** attempt to reposition within loops


## OSC CONTROLS

* `/play`:
    Start playback or restart if already playing

* `/stop`:
    Stop playback

* `/set` <string: parameter> <float: value>:
    Set parameter's value. Supported parameters:<br/>
    _tempo_: beats per minute<br/>
    _eighth_per_cycle_: cycle length in eigth notes. Loops lengths will always be a multiple of the cycle length.

* `/loop/#/hit` <string: command>:
    Apply _command_ to loop number _#_ (starting at 0). Loop Specifying _*_ will affect all loops. Also supports patterns like [1-3]. Supported commands:<br/>
    _record_: start/stop recording at next cycle beginning<br/>
    _overdub_: start/stop overdubbing immedietely<br/>
    _undo_: undo last overdub/record<br/>
    _redo_: redo last overdub/record<br/>
    _mute_on_: mute loop<br/>
    _mute_off_: unmute loop<br/>
    _clear_: clear loop and undo history

* `/status` <string: address>:
    Send looper's status as json<br/>
    _address_: *osc.udp://ip:port* or *osc.unix:///path/to/socket* ; if omitted the response will be sent to the sender

## OSC STATUS

<pre>

{
  "playing": <int>,
  "tick": <int>,
  "length": <int>,
  "tempo": <float>,
  "loops": [
    {
      "id": <int>,
      "length": <int>,
      "mute": <int>,
      "recording": <int>,
      "waiting": <int>,
      "record_stopping": <int>,
      "overdubbing": <int>
    },
    ...
  ]
}
</pre>


**Looper status**

    playing: playback state
    tick: playback tick (192 ticks = 1 quarter note)
    length: cycle length in ticks
    tempo: current bpm

**Loops statuses** (1 per loop)

    id: loop id (starting at 0)
    length: loop length in ticks (multiple of engine's length)
    mute: 1 if muted, 0 otherwise
    recording: 1 if recording, 0 otherwise
    waiting: 1 if waiting for next cycle to start/stop recording, 0 otherwise
    overdubbing: 1 if overdubbing, 0 otherwise


## AUTHORS

loop192 is written by Jean-Emmanuel Doucet.

## COPYRIGHT

Copyright © 2021 Jean-Emmanuel Doucet <jean-emmanuel@ammd.net>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

## LINKS

Sources: <a href="https://github.com/jean-emmanuel/loop192">https://github.com/jean-emmanuel/loop192</a>

<style type='text/css' media='all'>
/* style: toc */
.man-navigation {display:block !important;position:fixed;top:0;left:113ex;height:100%;width:100%;padding:48px 0 0 0;border-left:1px solid #dbdbdb;background:#eee}
.man-navigation a,.man-navigation a:hover,.man-navigation a:link,.man-navigation a:visited {display:block;margin:0;padding:5px 2px 5px 30px;color:#999;text-decoration:none}
.man-navigation a:hover {color:#111;text-decoration:underline}
</style>
