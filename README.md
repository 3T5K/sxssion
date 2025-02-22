# sxssion

Starts an X11 session with the specified environment (wraps startx).

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

## Building & Installing

```sh
cmake -S . -B build
cmake --build build --target sxssion
sudo install -m 755 ./builds/sxssion /usr/local/bin/
```
