[Português (PT-PT) →](./ARCHITECTURE.pt-PT.md)

# Architecture

## Goal

PlayerBots changes what the **MTA server browser** sees while keeping the server's real player model untouched.

```text
Lua resource
   │
   ├─ chooses virtual names
   ├─ computes virtual count
   ├─ handles replacement policy
   └─ sends count + names to the native module
            │
            ▼
Native module
   │
   ├─ hooks outgoing ASE sendto path
   ├─ recognizes EYE1 / EYE2 / EYE3
   ├─ rewrites the public count
   ├─ appends virtual names to EYE2
   └─ regenerates PingStatus for real + virtual
            │
            ▼
Unmodified MTA client browser
   ├─ parses EYE2
   ├─ runs UpdatePingStatus
   └─ keeps the advertised count because both values agree
```

## Why PingStatus matters

MTA's `CQueryReceiver` reads the EYE2 count and then calls the network interface's `UpdatePingStatus(...)`. During development, changing only EYE2 produced the expected value **before** that call, but the value was replaced with the real count afterwards.

The native implementation therefore uses the original PingStatus encoder from the loaded `net` module. It changes the encoder input player count to the public total instead of trying to invent the output bytes.

## EYE2 rewrite

The native code validates the expected EYE2 shape before rewriting anything:

- fixed `EYE2` prefix;
- six length-prefixed strings;
- binary password/serial/player/max bytes;
- textual `joined/max` metadata inside the map field;
- existing player-name entries;
- build number and PingStatus boundaries.

If a packet does not match the recognized format, it is forwarded unchanged.

## Name transport

The Lua module cannot pass arbitrary strings directly through the tiny historical MTA module registration interface used here, so names are transferred to C++ as hexadecimal nibbles through registered functions. This avoids adding a Lua C API dependency to the native module.

## Replacement policy

The Lua resource calculates:

```text
virtual = min(target, maxPlayers - realPlayers)
```

When replacement mode is enabled:

```text
virtual = min(virtual, replaceAt - realPlayers)
```

The selected name pool is the final upper bound.

## Failure model

Both native implementations are intentionally build-specific.

Windows validates:

- PE signature;
- timestamp;
- image size;
- checksum;
- short encoder instruction signature.

Linux validates:

- loaded `net.so` version through `GetLibMtaVersion`;
- short encoder instruction signature.

A failed validation leaves the encoder disabled instead of calling an unknown offset.
