[English →](./TROUBLESHOOTING.md)

# Resolução de problemas

## `/playerbots` não faz nada

Confirma que a conta tem o direito ACL:

```text
command.playerbots
```

O comando está registado como restrito.

## O painel abre, mas o módulo nativo aparece como indisponível

Confirma que o módulo está carregado no `mtaserver.conf` e que está na pasta correta para a arquitetura.

Windows x86:

```text
server/mods/deathmatch/modules/playerbots_native.dll
```

Linux x64:

```text
x64/modules/playerbots_native.so
```

Reinicia o servidor depois de substituir um módulo nativo.

## `encoder=NO` / a contagem pública continua a mostrar apenas jogadores reais

O teu `net.dll` / `net.so` provavelmente não corresponde à build testada. Executa o verificador incluído:

Windows:

```powershell
./verify_net.ps1 "C:\path\to\net.dll"
```

Linux:

```bash
./verify_net.sh /path/to/x64/net.so
```

Não removas a validação de versão/assinatura apenas para o obrigar a executar.

## O modo aleatório diz que não existem nomes suficientes

A versão open-source é distribuída intencionalmente com uma base vazia. Acrescenta nomes em:

```text
resource/playerbots/names.lua
```

ou utiliza nomes personalizados no painel.

## O modo de nomes específicos está vazio

A razão é a mesma: o `names.lua` começa vazio. O modo específico apenas apresenta os nomes existentes nesse ficheiro.

## Os nomes virtuais aparecem, mas a contagem está errada

Normalmente isto significa que a reescrita EYE2 está a funcionar, mas a sincronização do PingStatus não está. Confirma que o módulo nativo indica que o encoder está pronto e valida a build exata do módulo de rede MTA.

## Os jogadores reais ultrapassam o limite configurado para substituição

O PlayerBots nunca remove jogadores reais. Assim que a própria contagem de jogadores reais ultrapassar o limite de substituição, a contagem virtual passa para zero e a contagem pública acompanha a contagem real.

## O cache do browser parece desatualizado

Atualiza o browser de servidores ou volta a consultar o servidor. As respostas ASE são guardadas em cache pelo MTA durante pequenos intervalos, pelo que uma configuração acabada de alterar pode não ser visível no mesmo frame.

---

## Apoio e reporte de bugs

Para bugs, incompatibilidades de build ou dúvidas sobre a documentação:

- abre uma issue em [github.com/NexysT/MTA/issues](https://github.com/NexysT/MTA/issues) com `[PlayerBots]` no início do título;
- ou contacta **Nexys.Tuga** no Discord: `nobody_0101010101001`.

Para problemas do módulo nativo, inclui o sistema operativo, a arquitetura, a versão do MTA Server, o SHA-256 de `net.dll` / `net.so` e o output relevante da consola.
