[English →](./README.md)

# Módulo nativo Linux x64

Esta implementação destina-se ao módulo de rede MTA Linux 64-bit que foi testado.

Fingerprint testado:

```text
net.so version: 1.6.0-9.23312.0
SHA-256: 39d4a4f1b0b5c51ca792dc45f134c695e9083ea6ca266d61a2c3e4e085fcb56d
```

Compilação:

```bash
chmod +x build_linux_x64.sh
./build_linux_x64.sh
```

Instalação:

```text
x64/modules/playerbots_native.so
```

Carrega-o através do `mtaserver.conf`:

```xml
<module src="playerbots_native.so" />
```

Executa `verify_net.sh` antes de o instalar noutro servidor. Um hash diferente significa que o offset/ABI do encoder tem de ser portado e testado primeiro.
