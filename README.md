# sxssion

Starts an X11 session with the specified environment (wraps `startx`).

## Config

The configuration file will by default be looked up in `$XDG_CONFIG_HOME/sxssion/desktops.json`.

Expected format:
```json
{
    "desktop-1" : [
        "cmd-1",
        "cmd-2",
        ...
    ],

    "desktop-2" : [
        "cmd-1",
        "cmd-2",
        ...
    ],
    ...
}
```

Where `cmd-N` represents a command you'd normally put into your `~/.xinitrc`.

## Building & Installing

NOTE: Mare sure to initialize and update the git submodules or clone with `--recursive`.

```sh
cmake -S . -B build
cmake --build build --target sxssion
sudo install -m 755 ./build/sxssion /usr/local/bin/
```
