[Português (PT-PT) →](./README.pt-PT.md)

# Windows x86 native module

This implementation targets the tested 32-bit Windows MTA server network module.

Tested fingerprint:

```text
net.dll version: 1.6.0-9.24035.0
SHA-256: 4293afc3e1725c52d95409ee585b45ed487cd3d397ff7ff1bfdb4786bd067659
```

Build with Visual Studio C++ tools:

```text
build_x86.bat
```

Then install `build\playerbots_native.dll` in the x86 server module directory and load it from `mtaserver.conf`.

Run `verify_net.ps1` before deploying to another server. A different hash means the encoder offset must be ported and tested first.
