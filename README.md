# TLOU2 Viewmodel

A first-person viewmodel for **The Last of Us Part II Remastered** (PC). It works on top of the existing
TLOU2 First Person mod. That mod puts the camera at Ellie's eyes; this one makes her arms and the gun in her
hands sit in front of the camera, like in a first-person shooter.

- **Viewmodel:** her arms and the held gun are drawn with their own field of view, slightly nearer the
  camera, and held in view. Aiming, firing and reloading play in front of you.
- **Head lock:** while she vaults, climbs, hangs on a ledge or uses a ladder, the camera can only turn
  about 65° either side of where her body faces. The edge is soft, and the lock eases in and out.
- Nothing in the game folder is changed. Everything loads from its own folder into the running game.

This is version 0.1.0, an early release. See the known issues below.

## Requirements

- The Last of Us Part II Remastered on PC (Steam build of 2025-04-03, `tlou-ii.exe` timestamp 67ED75F7).
  Other builds are refused safely: the DLL checks the game code before it hooks anything.
- The **TLOU2 First Person** mod (`TLOU2FirstPerson.asi`, tested with v1.7.2), by its own author. It is not
  included here; get it from where it is published (Nexus Mods).
- **Turn Depth of Field off** in the game's Graphics settings. With it on, the game blurs the arms and gun
  because they are close to the camera.

## Install

1. Put the first-person mod's `TLOU2FirstPerson.asi` and `TLOU2FirstPerson.ini` in a folder of their own
   (not the game folder).
2. Unzip this release into the same folder: `TLOU2Viewmodel.dll`, `TLOU2Viewmodel.ini`, `fp_launcher.exe`,
   `Launch.cmd`.
3. Start the game normally (not as administrator).
4. Run `Launch.cmd`. It waits for the game window, loads the first-person mod, then this mod. Leave it open:
   it loads both again if you restart the game.
5. In game, press **F5** (the first-person mod's key) for first person.

## Settings

`TLOU2Viewmodel.ini` is read once when the mod loads. While you play, you can write the same commands into
`TLOU2Viewmodel.cmd.txt` in the same folder; the mod reads that file every 0.2 s and empties it. It logs to
`TLOU2Viewmodel.log`.

| Command | What it does |
|---|---|
| `arms <fov> <s>` | Turn the viewmodel on: arms and gun drawn as if seen with a `<fov>`° field of view, at `<s>` times their real distance (default `arms 60 0.75`) |
| `hold <right> <up> <forward>` | Where the arms and gun sit, in metres. Negative `right` moves them to the right (default `hold -0.09 0.04 0.3`) |
| `weapon on` / `weapon off` | Whether the held gun follows the arms |
| `armroot clavicle` / `armroot shoulder` | Where the moved arm starts |
| `off` | Turn the viewmodel off |
| `headlock <degrees>` | Head-turn limit during vaults and climbs (default 65; `0` = off) |
| `headlock soft <edge> <in> <out>` | Soft edge width (°), ease-in and ease-out speed (°/s); default `12 180 300` |
| `headlock always` / `headlock moves` | Limit in every state (test) / only during those moves |

## Known issues

- The rifle sling does not line up with the moved rifle. It is a separate object that is not moved yet.
- When you look far up or down, the gun follows the game's third-person aim pose, which pitches less than the
  camera, so it can point slightly off.
- Depth of Field must be off in the Graphics settings (see above).
- Load order matters: this mod must load after the first-person mod. `Launch.cmd` does that.
- Only the local player is changed.

## How it works

The game skins every character on the GPU from one 3x4 matrix per joint. The mod changes the render copy
of those matrices for Ellie's arms (both clavicles and everything below them, including the procedural
helper joints) and for the weapon in her hands. Her real pose, position and collision stay untouched.

The change scales the arms and gun about the eye: by `k * s` across the view and by `s` along it. Seen from
the eye, that looks like a different field of view (`k = tan(world fov / 2) / tan(viewmodel fov / 2)`) at
`s` times the distance. A camera-space offset (`hold`) puts them in view. The math is in
`src/core/viewmodel.h` and is unit-tested.

The head lock turns the incoming camera rotation about the vertical axis, before the first-person mod
builds its view from it.

## Build

Visual Studio 2022 Build Tools (C++) and its bundled CMake/Ninja: run `build.cmd`. It builds
`build\TLOU2Viewmodel.dll` and `build\fp_launcher.exe` and runs the tests.

## Credits and notes

- Camera and head hiding: the TLOU2 First Person mod by its author. This mod only adds the viewmodel and the
  head lock on top.
- Built with the help of AI (Claude Code by Anthropic): the reverse engineering, the code and the tests.
- No game files are included or modified. Use it in single player only.

## License

MIT. See `LICENSE`.
