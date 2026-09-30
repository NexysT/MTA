[English →](./ARCHITECTURE.md)

# Arquitetura

## Objetivo

O PlayerBots altera aquilo que o **browser de servidores MTA** vê, mantendo intacto o modelo real de jogadores do servidor.

```text
Resource Lua
   │
   ├─ escolhe nomes virtuais
   ├─ calcula a quantidade virtual
   ├─ gere a política de substituição
   └─ envia quantidade + nomes para o módulo nativo
            │
            ▼
Módulo nativo
   │
   ├─ interceta o caminho de saída ASE em sendto
   ├─ reconhece EYE1 / EYE2 / EYE3
   ├─ reescreve a contagem pública
   ├─ acrescenta nomes virtuais ao EYE2
   └─ regenera PingStatus para reais + virtuais
            │
            ▼
Browser de um cliente MTA não modificado
   ├─ interpreta EYE2
   ├─ executa UpdatePingStatus
   └─ mantém a contagem anunciada porque ambos os valores coincidem
```

## Porque é que o PingStatus é importante

O `CQueryReceiver` do MTA lê a contagem EYE2 e depois chama `UpdatePingStatus(...)` da interface de rede. Durante o desenvolvimento, alterar apenas o EYE2 produzia o valor esperado **antes** dessa chamada, mas o valor era substituído pela contagem real depois dela.

Por isso, a implementação nativa utiliza o encoder original de PingStatus do módulo `net` carregado. Em vez de tentar inventar os bytes de saída, altera a contagem de jogadores fornecida ao encoder para o total público.

## Reescrita do EYE2

O código nativo valida o formato EYE2 esperado antes de reescrever qualquer coisa:

- prefixo fixo `EYE2`;
- seis strings com prefixo de comprimento;
- bytes binários de password/serial/jogadores/máximo;
- metadados textuais `joined/max` dentro do campo de mapa;
- entradas de nomes de jogadores já existentes;
- número de build e limites do PingStatus.

Se um pacote não corresponder ao formato reconhecido, é reenviado sem alterações.

## Transporte dos nomes

O módulo Lua não consegue passar strings arbitrárias diretamente através da pequena interface histórica de registo de módulos MTA utilizada aqui, por isso os nomes são transferidos para C++ como nibbles hexadecimais através de funções registadas. Isto evita acrescentar uma dependência da API C de Lua ao módulo nativo.

## Política de substituição

O resource Lua calcula:

```text
virtual = min(target, maxPlayers - realPlayers)
```

Quando o modo de substituição está ativado:

```text
virtual = min(virtual, replaceAt - realPlayers)
```

O conjunto de nomes selecionado funciona como limite superior final.

## Modelo de falha

As duas implementações nativas são intencionalmente específicas de cada build.

O Windows valida:

- assinatura PE;
- timestamp;
- tamanho da imagem;
- checksum;
- pequena assinatura de instruções do encoder.

O Linux valida:

- versão do `net.so` carregado através de `GetLibMtaVersion`;
- pequena assinatura de instruções do encoder.

Se uma validação falhar, o encoder fica desativado em vez de chamar um offset desconhecido.
