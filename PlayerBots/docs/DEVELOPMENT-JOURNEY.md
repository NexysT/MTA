# Development journey

This document records the technical path that led to the current PlayerBots design. It is included because the difficult part was not adding names to a packet; it was understanding why the stock MTA browser rejected an apparently correct player count.

## 1. First useful result: names appeared, count did not

The first native versions intercepted ASE replies and added virtual names to EYE2. The player list in the stock browser showed the virtual entries, proving the packet hook was working.

The public count still showed only real players.

That split was the first important clue: the browser was not deriving its displayed count from the number of names in the list.

## 2. EYE2 count was rewritten in both places

MTA EYE2 carries the player count twice:

- a binary player-count byte;
- a textual `joined/max` value embedded in the metadata field.

Both were rewritten and kept coherent. A standalone UDP query confirmed the reply contained the virtual total and all expected names.

The stock browser still corrected the count.

## 3. A diagnostic client located the overwrite

A debug build of the open-source MTA client was instrumented around `CQueryReceiver` and `CServerList`.

The resulting log showed the decisive transition:

```text
QUERY PRE  players=12 max=32 verified=1
QUERY POST players=1  max=32 verified=1
QUERY NAMES count=12 players=1
```

The count was correct immediately after EYE2 parsing and changed during `UpdatePingStatus`.

This ruled out the UI layer and the server-list drawing code.

## 4. Removing PingStatus was not a solution

A test build removed the PingStatus block while leaving the virtual count intact.

The client then reported:

```text
QUERY PRE  players=12 max=32 verified=1
QUERY POST players=1  max=32 verified=0
```

That confirmed PingStatus was part of the validation/recovery mechanism. The correct solution was not to remove it.

## 5. Capturing real PingStatus samples

The next test captured the original PingStatus bytes with different real-player states. The bytes changed between requests even when the real player count stayed the same.

That proved the block was not a simple fixed lookup such as "1 player = these bytes".

## 6. Reusing the original encoder

Static analysis of the Windows `net.dll` located the internal encoder reached by the server's `GetPingStatus` path.

Instead of reverse-engineering the encoded format itself, the native module called the **original encoder** with a different player-count input:

```text
real players + virtual players
```

After replacing the PingStatus field with that generated value, the diagnostic client showed:

```text
QUERY PRE  players=12 max=32 verified=1
QUERY POST players=12 max=32 verified=1
LIST POST  nPlayers=12 max=32 names=12 verified=1
```

The same stock MTA client then showed 12/32 with no real players, 13/32 with one real player plus 12 virtual players, and 29/32 with one real player plus 28 virtual players.

## 7. Linux x64 port

The production-style test server used Linux x64, where the network module is `x64/net.so`, not `net.dll`.

The Linux port required:

- a new encoder ABI for libstdc++ `std::string`;
- locating and validating the Linux encoder offset;
- PLT/GOT interception of `sendto` in `deathmatch.so` instead of Windows IAT patching;
- x64 module placement under `x64/modules/`.

The public source keeps Windows and Linux implementations separate because the loader, ABI and hook mechanisms are different.

## 8. Public release cleanup

The public release deliberately removed project-specific branding and the original internal name list. It ships with an empty `names.lua` and all source code required to inspect, modify and rebuild the resource and native modules.

## Lessons

- A packet that looks correct on the wire can still be revalidated by a later client subsystem.
- Instrumenting the official client source was more useful than repeatedly guessing packet bytes.
- Reusing the original encoder was safer and more maintainable than cloning an opaque format from samples.
- Build fingerprints and signature guards are essential when a native module calls version-specific internal code.
