<p align="center">
  <img src="./assets/playerbots-banner.svg" alt="PlayerBots para MTA:SA" width="100%">
</p>

<h1 align="center">PlayerBots</h1>

<p align="center">Jogadores virtuais no browser público do <strong>Multi Theft Auto: San Andreas</strong>, em código aberto, por <strong>Nexys.Tuga</strong>.</p>

O PlayerBots permite apresentar uma quantidade configurável de jogadores virtuais no browser do MTA. A contagem pública, a lista de nomes e a validação `PingStatus` ficam coerentes, pelo que clientes MTA normais conseguem ver o mesmo resultado sem instalar um cliente modificado.

O sistema **não cria ligações reais**, não adiciona jogadores a `getElementsByType("player")` e não ocupa sessões de rede reais. A alteração é feita nas respostas ASE usadas pelo browser.

> **Importante:** os módulos nativos estão presos às builds de `net.dll` / `net.so` que foram testadas. Se a tua build for diferente, é necessário portar e testar o encoder antes de usar.

[English README →](./README.md)

---

## O que inclui

- Contagem pública = jogadores reais + jogadores virtuais.
- Nomes virtuais visíveis na lista de jogadores do browser.
- Três modos de nomes: aleatórios da base, específicos da base ou personalizados por vírgulas.
- `names.lua` vazio por defeito para cada servidor colocar apenas os seus próprios nomes.
- Modo de substituição dinâmica: jogadores reais substituem bots um a um a partir de um limite configurável.
- Painel CEF através de `/playerbots`.
- Comando protegido por ACL através de `command.playerbots`.
- API para scoreboards e outros resources.
- Código-fonte C++ para Windows x86 e Linux x64.
- Sem telemetria ou serviços externos.

## Exemplo do modo substituir

Se configurares 20 bots e definires «substituir aos 25», com 5 jogadores reais tens `5 + 20 = 25`. Entra mais um jogador real: fica `6 + 19 = 25`. Entra outro: `7 + 18 = 25`. Quando um real sai, o bot correspondente pode regressar até ao máximo configurado.

---

## Instalação rápida

1. Copia `resource/playerbots/` para os resources do servidor.
2. Compila o módulo da tua plataforma.
3. Coloca o binário na pasta de módulos correta.
4. Adiciona o módulo ao `mtaserver.conf`.
5. Reinicia o servidor.
6. Dá `command.playerbots` ao grupo ACL que pode gerir o painel.
7. Usa `/playerbots`.

### Windows x86

Compila em `platforms/windows-x86/`:

```text
build_x86.bat
```

Resultado:

```text
build\playerbots_native.dll
```

Destino normal:

```text
server\mods\deathmatch\modules\playerbots_native.dll
```

No `mtaserver.conf`:

```xml
<module src="playerbots_native.dll" />
```

Build `net.dll` validada:

```text
1.6.0-9.24035.0
SHA-256 4293afc3e1725c52d95409ee585b45ed487cd3d397ff7ff1bfdb4786bd067659
```

### Linux x64

Compila em `platforms/linux-x64/`:

```bash
chmod +x build_linux_x64.sh
./build_linux_x64.sh
```

Resultado:

```text
build/playerbots_native.so
```

Destino:

```text
x64/modules/playerbots_native.so
```

No `mtaserver.conf`:

```xml
<module src="playerbots_native.so" />
```

Build `net.so` validada:

```text
1.6.0-9.23312.0
SHA-256 39d4a4f1b0b5c51ca792dc45f134c695e9083ea6ca266d61a2c3e4e085fcb56d
```

---

## Base de nomes

A versão pública começa vazia:

```lua
PLAYERBOTS_NAMES = {
}
```

Podes acrescentar os teus próprios nomes:

```lua
PLAYERBOTS_NAMES = {
    "Alex Silva",
    "Jordan Miles",
    "Sam Costa",
}
```

No painel também podes escolher **Nomes personalizados** e escrever:

```text
Carlos Pereira, Carlos Dias, Artur_Jorge
```

---

## API

```lua
exports.playerbots:getPublicPlayerCount()
exports.playerbots:getVirtualPlayerCount()
exports.playerbots:getVirtualPlayers()
exports.playerbots:isPlayerBotsEnabled()
```

Element data disponibilizada no `root`:

```text
playerbots:enabled
playerbots:realPlayers
playerbots:virtualPlayers
playerbots:publicPlayers
playerbots:maxPlayers
playerbots:bots
playerbots
```

---

## Porque é que não bastava alterar o número do EYE2?

Durante o desenvolvimento, o pacote EYE2 já chegava ao cliente com a quantidade virtual correta e os nomes corretos. Mesmo assim, o browser voltava a apresentar apenas os jogadores reais. Um cliente de diagnóstico permitiu observar:

```text
QUERY PRE  players=12
QUERY POST players=1
```

O valor era substituído em `UpdatePingStatus`. A solução final foi manter a resposta ASE inteira coerente e reutilizar o encoder original do módulo de rede MTA para produzir um `PingStatus` válido para `reais + virtuais`.

A documentação completa está em:

- [Arquitetura](./docs/ARCHITECTURE.md)
- [Percurso de desenvolvimento](./docs/DEVELOPMENT-JOURNEY.md)
- [Resolução de problemas](./docs/TROUBLESHOOTING.md)

---

## Licença

MIT. Consulta [LICENSE](./LICENSE).

Multi Theft Auto é um projeto separado. PlayerBots é um projeto comunitário independente e não é afiliado nem oficialmente suportado pela equipa do MTA.
