# loop192

Minimal MIDI live looper that works like sooperlooper but with MIDI instead of audio.

![loop192 window](https://user-images.githubusercontent.com/5261671/140610503-493f5505-9a6e-4e40-b0d7-64b7ddc918b0.png)

## Build

**Dependencies** (as debian packages)
```
libjack-jackd2-dev liblo-dev libgtkmm-3.0-dev libasound2-dev
```

**Build**
```
make clean && make -j8
```

**Run**

```
Usage: ./src/loop192 [options...]
Options:
  -l <int> , --loops=<int>                             number of midi loops (default: 8)
  -p <str> , --osc-port=<str>                          udp in port number or unix socket path
                                                       for OSC server
  -r [<int> ...] , --release-controls=[<int> ...]      list of control numbers separated by
                                                       spaces that should be reset to 0 when
                                                       muting a loop or stopping transport
  -j , --jack-transport                                follow jack transport
  -n , --no-gui                                        headless mode
  -h , --help                                          this usage output
  -v , --version                                       show version only
```

**Install**

```bash
sudo make install
```

Append `PREFIX=/usr` to override the default installation path (`/usr/local`)

**Uninstall**

```bash
sudo make uninstall
```

Append `PREFIX=/usr` to override the default uninstallation path (`/usr/local`)

## Documentation

See [loop192.ammd.net](https://loop192.ammd.net/) or run `man loop192` after installing.
