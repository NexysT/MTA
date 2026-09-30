<p align="center">
  <img src="./assets/playerbots-banner.svg" alt="PlayerBots for MTA:SA" width="100%">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/MTA%3ASA-1.6-36d6a6?style=flat-square&labelColor=102722" alt="MTA:SA 1.6">
  <img src="https://img.shields.io/badge/PLATFORMS-Windows_x86_%7C_Linux_x64-7aabeb?style=flat-square&labelColor=102722" alt="Windows x86 and Linux x64">
  <img src="https://img.shields.io/badge/LICENSE-MIT-36d6a6?style=flat-square&labelColor=102722" alt="MIT License">
  <img src="https://img.shields.io/badge/SOURCE-Lua_%2B_C%2B%2B-7aabeb?style=flat-square&labelColor=102722" alt="Lua and C++">
</p>

<h1 align="center">PlayerBots</h1>

<p align="center">
  Open-source virtual players for the <strong>Multi Theft Auto: San Andreas</strong> server browser.<br>
  Built by <strong>Nexys.Tuga</strong>.
</p>

PlayerBots lets a server advertise a configurable set of virtual players in the MTA server browser. The browser count, the visible player-name list and the internal MTA `PingStatus` verification are kept consistent, so ordinary MTA clients can see the same public count without installing a custom client.

It does **not** create real player connections, does not add elements to `getElementsByType("player")`, and does not occupy actual network sessions. It changes the ASE query replies used by the browser.

> **Important:** the native modules are deliberately fail-closed and tied to tested `net.dll` / `net.so` builds. Do not reuse the hard-coded encoder offset on a different MTA build without porting and testing it first.

[Português (PT-PT) →](./README.pt-PT.md)

---

## Features

- Public browser count: real players + virtual players.
- Virtual names shown in the browser's player list.
- Three name modes:
  - random names from `names.lua`;
  - selected names from `names.lua`;
  - comma-separated custom names from the panel.
- Empty public name database by default: add only the names you want to use.
- Dynamic replacement mode: real players can replace virtual players one by one after a configured total is reached.
- CEF administration panel opened with `/playerbots`.
- ACL-protected command: `command.playerbots`.
- Exported server API for scoreboards or other resources.
- Windows x86 and Linux x64 native implementations included as source.
- No telemetry or external service dependency.

## Example replacement behaviour

Configuration:

```text
Virtual target: 20
Replace at total: 25
```

With 5 real players the public count is 25 (`5 + 20`). When a sixth real player joins, one virtual player disappears, so the public count remains 25 (`6 + 19`). If another real player joins, it becomes `7 + 18`, and so on. When real players leave, virtual players are restored automatically up to the configured target.

---

## Repository layout

```text
PlayerBots/
├── README.md
├── README.pt-PT.md
├── LICENSE
├── assets/
│   └── playerbots-banner.svg
├── resource/
│   └── playerbots/
│       ├── meta.xml
│       ├── names.lua
│       ├── server.lua
│       ├── client.lua
│       └── ui/
│           ├── index.html
│           ├── app.js
│           └── style.css
├── platforms/
│   ├── windows-x86/
│   │   ├── playerbots_native_windows_x86.cpp
│   │   ├── build_x86.bat
│   │   └── verify_net.ps1
│   └── linux-x64/
│       ├── playerbots_native_linux_x64.cpp
│       ├── build_linux_x64.sh
│       └── verify_net.sh
└── docs/
    ├── ARCHITECTURE.md
    ├── DEVELOPMENT-JOURNEY.md
    └── TROUBLESHOOTING.md
```

---

## Compatibility

### Windows x86

The Windows implementation was tested against this exact `net.dll` fingerprint:

```text
MTA net version: 1.6.0-9.24035.0
SHA-256: 4293afc3e1725c52d95409ee585b45ed487cd3d397ff7ff1bfdb4786bd067659
PE TimeDateStamp: 0x6AADC98B
SizeOfImage: 0x00221000
Checksum: 0x002036FC
```

The module checks the PE fingerprint and instruction signature before using the encoder. If it does not match, the encoder remains disabled.

### Linux x64

The Linux implementation was tested against:

```text
MTA net version: 1.6.0-9.23312.0
SHA-256: 39d4a4f1b0b5c51ca792dc45f134c695e9083ea6ca266d61a2c3e4e085fcb56d
```

The module checks `GetLibMtaVersion` and a short instruction signature before using the encoder.

If your build differs, see [Porting to another MTA build](#porting-to-another-mta-build).

---

## Installation

### 1. Install the resource

Copy:

```text
resource/playerbots/
```

to your MTA resources directory and keep the resource name as:

```text
playerbots
```

Then add/start it as usual:

```text
refresh
start playerbots
```

### 2. Build and install the native module

#### Windows x86

Requirements:

- Visual Studio with the C++ desktop workload.
- x86 MSVC build tools.

Open a Developer Command Prompt in:

```text
platforms/windows-x86/
```

and run:

```text
build_x86.bat
```

Output:

```text
build\playerbots_native.dll
```

Copy it to the x86 module directory, normally:

```text
server\mods\deathmatch\modules\playerbots_native.dll
```

#### Linux x64

Requirements:

```text
g++
libdl
```

Run:

```bash
cd platforms/linux-x64
chmod +x build_linux_x64.sh
./build_linux_x64.sh
```

Output:

```text
build/playerbots_native.so
```

Copy it to:

```text
x64/modules/playerbots_native.so
```

### 3. Load the module

Add the appropriate module to `mtaserver.conf`:

Windows:

```xml
<module src="playerbots_native.dll" />
```

Linux:

```xml
<module src="playerbots_native.so" />
```

Restart the MTA server after changing native modules.

### 4. Grant panel access

The command is restricted by MTA ACL using:

```text
command.playerbots
```

Grant that right only to the ACL groups that should be allowed to configure PlayerBots.

### 5. Add names

The public version intentionally ships with an empty `names.lua`:

```lua
PLAYERBOTS_NAMES = {
}
```

Add your own entries:

```lua
PLAYERBOTS_NAMES = {
    "Alex Silva",
    "Jordan Miles",
    "Sam Costa",
}
```

Alternatively, select **Custom names** in the panel and enter names separated by commas.

---

## Panel

Run:

```text
/playerbots
```

The panel lets an authorized administrator configure:

- enabled/disabled state;
- virtual-player target;
- random, specific or custom names;
- dynamic replacement threshold;
- whether the virtual list is exposed through element data for compatible scoreboards.

Settings are stored in `settings.json` inside the resource's private writable storage.

---

## Server API

The resource exports:

```lua
exports.playerbots:getPublicPlayerCount()
exports.playerbots:getVirtualPlayerCount()
exports.playerbots:getVirtualPlayers()
exports.playerbots:isPlayerBotsEnabled()
```

It also publishes these root element-data keys:

```text
playerbots:enabled
playerbots:realPlayers
playerbots:virtualPlayers
playerbots:publicPlayers
playerbots:maxPlayers
playerbots:bots
playerbots
```

Example:

```lua
local total = exports.playerbots:getPublicPlayerCount()
local bots = exports.playerbots:getVirtualPlayers()
```

---

## How it works

The MTA browser receives ASE query replies. Updating only the visible EYE2 count is not enough: the client later calls `UpdatePingStatus`, which verifies/reconstructs the player count from the server's `PingStatus` block.

PlayerBots therefore keeps three parts coherent:

1. the numeric ASE player count;
2. the player-name list;
3. a `PingStatus` generated by the original encoder from the loaded MTA network module, but using the public total (`real + virtual`).

The native module hooks the outgoing ASE `sendto` path, rewrites only recognized EYE reply formats and leaves unknown packets untouched.

See [Architecture](./docs/ARCHITECTURE.md) for the detailed flow.

---

## Porting to another MTA build

Do **not** blindly change the hard-coded encoder RVA.

A proper port should:

1. identify the exact `net.dll` / `net.so` version and SHA-256;
2. locate the `GetPingStatus` encoder path for that build;
3. determine the encoder ABI and output string layout;
4. add a strict fingerprint/signature guard;
5. test with 0, 1 and multiple real players;
6. verify the final result with an unmodified MTA client;
7. confirm that real-player joins and quits keep the public count coherent.

The existing platform sources are useful references, not universal offsets.

---

## Development notes

This project took several iterations because the first implementation looked correct at packet level but the stock browser still corrected the displayed count back to the real number. A diagnostic client build showed the exact transition:

```text
QUERY PRE  players=12
QUERY POST players=1
```

That narrowed the problem to `UpdatePingStatus`. The final approach stopped forging only the EYE2 count and instead re-used the original MTA network encoder to create a matching `PingStatus` for the public total.

The full technical timeline is documented in [Development journey](./docs/DEVELOPMENT-JOURNEY.md).

---

## Security and operational notes

- Keep backups before testing a native module on a production server.
- A mismatched native offset can crash a server; the included builds use version/signature guards for this reason.
- Do not expose passwords, API keys or other secrets in a resource repository.
- PlayerBots changes server-browser presentation only; it should not be used as a substitute for real server activity metrics.

---

## License

MIT. See [LICENSE](./LICENSE).

Multi Theft Auto is a separate project. PlayerBots is an independent community project and is not affiliated with or endorsed by the MTA team.
