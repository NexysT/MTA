<picture>
  <source media="(max-width: 600px)" srcset="./assets/mta-banner-mobile.svg">
  <img src="./assets/mta-banner.svg" alt="MTA:SA — NexysT. Native modules e Lua resources." width="100%">
</picture>



[Português](./README.md) · [English](./README.en.md)

# MTA:SA — resources and native integration

Projects by **NexysT** for **Multi Theft Auto: San Andreas**, with documentation and files organized by resource. The main project is **PlayerBots**: native C++ modules and Lua integration with the server browser, alongside an administration panel adapted for MTA Portugal.

## PlayerBots

### [Native integration with the MTA:SA browser →](./PlayerBots/)

PlayerBots presents virtual players in the public MTA browser without creating real server connections. It keeps the public count, player-name list and `PingStatus` used by the stock client consistent.

- **C++ + Lua:** native modules, resource and CEF panel.
- **Windows x86 / Linux x64:** separate implementations, hooking `sendto` through IAT or PLT/GOT.
- **Low-level analysis and debugging:** investigation of ASE/EYE2 and `UpdatePingStatus` verification, MTA client instrumentation and reuse of the original network encoder.
- **Configuration:** random, specific or custom names; real players can replace virtual players one by one through dynamic mode.
- **Code and documentation:** source code, platform build scripts and technical documentation covering architecture, compatibility and porting.

> The native modules target tested `net.dll` / `net.so` builds. A different build requires porting and testing; version/fingerprint and signature checks protect access to the encoder.

**[Code, compatibility and installation](./PlayerBots/)** · [Architecture](./PlayerBots/docs/ARCHITECTURE.md) · [Development journey](./PlayerBots/docs/DEVELOPMENT-JOURNEY.md)

## Administration panel

### [Administration Panel | MTA Portugal →](./Painel-Administracao-MTA-Portugal/)

An adapted version of MTA's `admin` resource. Its interface was translated and tools were added for server management and development:

- Portuguese interface with menus and buttons adjusted to the panel's available space.
- Player information, administrative actions, resources, maps, bans and server options.
- **Debugging:** a tab for following warnings, errors and messages received by the panel.
- **Commands:** select a resource and inspect its registered commands.
- **ACL:** visual management of roles, accounts and permissions.
- **Anonymous admin:** public messages do not identify who performs the covered actions; the internal log retains the administrator's identity.

**[Features, installation and permissions →](./Painel-Administracao-MTA-Portugal/)**

## Download and install

Start with the page for the project you want to try:

- [PlayerBots](./PlayerBots/): source code, native-module builds, platform compatibility and resource installation.
- [Administration Panel](./Painel-Administracao-MTA-Portugal/): structure, installation and permission considerations.

On GitHub, **Code → Download ZIP** downloads **the entire repository**. To install the panel, extract the ZIP and use:

```text
Painel-Administracao-MTA-Portugal/admin/
```

The resource remains named `admin` on the server; do not rename it to “Administration Panel”. For PlayerBots, follow its own build and installation steps on the project page.

### Compiled panel scripts

The published panel scripts were prepared in compiled form and retained the `.lua` extension to preserve the paths in `meta.xml`. The file extension alone does not indicate whether the contents are readable source code.

Compilation and obfuscation make reading and directly reusing the code more difficult, but do not guarantee that it cannot be analyzed. Do not publish passwords, tokens or other secrets inside a resource, even when the scripts are compiled.

## Organization and origins

```text
MTA/
├── README.md
├── README.en.md
├── assets/
│   └── mta-banner.svg
├── PlayerBots/
│   ├── README.md
│   ├── README.pt-PT.md
│   ├── LICENSE
│   ├── resource/playerbots/
│   ├── platforms/
│   │   ├── windows-x86/
│   │   └── linux-x64/
│   └── docs/
└── Painel-Administracao-MTA-Portugal/
    ├── README.md
    ├── LICENSE-MTA.txt
    └── admin/
        ├── meta.xml
        ├── admin_definitions.lua
        ├── client/
        ├── server/
        ├── conf/
        └── ...
```

Each resource keeps its documentation and files in its own folder. New resources sit alongside the existing ones, without mixing their code.

The panel derives from the `admin` resource in the [official mtasa-resources project](https://github.com/multitheftauto/mtasa-resources). Credits and the original MIT license are on the [panel page](./Painel-Administracao-MTA-Portugal/). The modifications, translation and new tools were developed for **MTA Portugal (NexysT)**.

PlayerBots is an independent community project with an [MIT license](./PlayerBots/LICENSE) and its own documentation. It is not affiliated with or endorsed by the MTA team.

[GitHub profile — NexysT ↗](https://github.com/NexysT)

