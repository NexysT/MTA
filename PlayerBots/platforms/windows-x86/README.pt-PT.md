[English →](./README.md)

# Módulo nativo Windows x86

Esta implementação destina-se ao módulo de rede do MTA Server Windows 32-bit que foi testado.

Fingerprint testado:

```text
net.dll version: 1.6.0-9.24035.0
SHA-256: 4293afc3e1725c52d95409ee585b45ed487cd3d397ff7ff1bfdb4786bd067659
```

Compila com as ferramentas C++ do Visual Studio:

```text
build_x86.bat
```

Depois instala `build\playerbots_native.dll` na pasta de módulos do servidor x86 e carrega-o através do `mtaserver.conf`.

Executa `verify_net.ps1` antes de o instalar noutro servidor. Um hash diferente significa que o offset do encoder tem de ser portado e testado primeiro.
