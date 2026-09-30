# Troubleshooting

## `/playerbots` does nothing

Check that the account has the ACL right:

```text
command.playerbots
```

The command is registered as restricted.

## Panel opens but the native module says unavailable

Confirm the module is loaded in `mtaserver.conf` and is in the correct architecture-specific folder.

Windows x86:

```text
server/mods/deathmatch/modules/playerbots_native.dll
```

Linux x64:

```text
x64/modules/playerbots_native.so
```

Restart the server after replacing a native module.

## `encoder=NO` / public count stays real-only

Your `net.dll` / `net.so` probably does not match the tested build. Run the supplied verifier:

Windows:

```powershell
./verify_net.ps1 "C:\path\to\net.dll"
```

Linux:

```bash
./verify_net.sh /path/to/x64/net.so
```

Do not remove the version/signature guard just to force it to run.

## Random mode says there are not enough names

The open-source version intentionally ships with an empty database. Add names to:

```text
resource/playerbots/names.lua
```

or use custom names in the panel.

## Specific-name mode is empty

Same reason: `names.lua` starts empty. Specific mode only lists names from that file.

## Virtual names appear but the count is wrong

That normally means the EYE2 rewrite is working but PingStatus synchronization is not. Confirm the native module reports the encoder as ready and verify the exact MTA network-module build.

## Real players exceed the configured replacement threshold

PlayerBots never removes real players. Once the real-player count itself exceeds the replacement threshold, virtual count becomes zero and the public count follows the real count.

## The browser cache looks stale

Refresh the server browser or query the server again. ASE replies are cached by MTA for short intervals, so a just-changed configuration may not be visible in the same frame.
