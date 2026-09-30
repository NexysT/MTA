# Linux x64 native module

This implementation targets the tested 64-bit Linux MTA network module.

Tested fingerprint:

```text
net.so version: 1.6.0-9.23312.0
SHA-256: 39d4a4f1b0b5c51ca792dc45f134c695e9083ea6ca266d61a2c3e4e085fcb56d
```

Build:

```bash
chmod +x build_linux_x64.sh
./build_linux_x64.sh
```

Install:

```text
x64/modules/playerbots_native.so
```

Load from `mtaserver.conf`:

```xml
<module src="playerbots_native.so" />
```

Run `verify_net.sh` before deploying to another server. A different hash means the encoder offset/ABI must be ported and tested first.
