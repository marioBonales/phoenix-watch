# phoenix-watchface

A Pebble watchface that shows battery level and steps progress 

# Screenshots

<img width="200" alt="phoenix-watch-live" src="https://github.com/user-attachments/assets/ef994a25-8bfd-436f-83a0-b677b6bf347b" />  

<br/> <br />

<img width="200" height="260" alt="phoenix-demo" src="https://github.com/user-attachments/assets/c399529d-fef4-4935-b66d-fa15efe2b3b6" />


## Building & running

```sh
pebble build                          # build for all targetPlatforms
pebble install --emulator emery       # install on the emery emulator
pebble install --phone <ip>           # install to a paired phone
```

## Target platforms

`targetPlatforms` in `package.json` controls which watches you build for. The
modern Pebble hardware is **emery** (Pebble Time 2), **gabbro** (Pebble Round
2), and **flint** (Pebble 2 Duo); the original Pebble platforms (aplite,
basalt, chalk, diorite) are included by default for backwards compatibility.

## Project layout

```
src/c/           C source for the watchapp
src/pkjs/        PebbleKit JS (phone-side) source, if any
worker_src/c/    Background worker source, if any
resources/       Images, fonts, and other bundled resources
package.json     Project metadata (UUID, platforms, resources, message keys)
wscript          Build rules — usually no need to edit
```

By default this project is configured as a watchapp. To make it a watchface,
set `pebble.watchapp.watchface` to `true` in `package.json`.

## Documentation

Full SDK docs, tutorials, and API reference: <https://developer.repebble.com>
