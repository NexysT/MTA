[English →](./DEVELOPMENT-JOURNEY.md)

# Percurso de desenvolvimento

Este documento regista o caminho técnico que levou ao desenho atual do PlayerBots. Está incluído porque a parte difícil não foi acrescentar nomes a um pacote; foi perceber porque é que o browser original do MTA rejeitava uma contagem de jogadores que, aparentemente, estava correta.

## 1. Primeiro resultado útil: os nomes apareciam, a contagem não

As primeiras versões nativas intercetavam as respostas ASE e acrescentavam nomes virtuais ao EYE2. A lista de jogadores do browser original mostrava as entradas virtuais, provando que o hook dos pacotes estava a funcionar.

A contagem pública continuava a mostrar apenas os jogadores reais.

Essa diferença foi a primeira pista importante: o browser não estava a calcular a contagem apresentada a partir da quantidade de nomes existentes na lista.

## 2. A contagem EYE2 foi reescrita nos dois locais

O EYE2 do MTA transporta a contagem de jogadores duas vezes:

- um byte binário com a contagem de jogadores;
- um valor textual `joined/max` incorporado no campo de metadados.

Os dois foram reescritos e mantidos coerentes. Uma consulta UDP independente confirmou que a resposta continha o total virtual e todos os nomes esperados.

Mesmo assim, o browser original continuava a corrigir a contagem.

## 3. Um cliente de diagnóstico localizou a substituição

Foi instrumentada uma build Debug do cliente MTA open-source em torno de `CQueryReceiver` e `CServerList`.

O log resultante mostrou a transição decisiva:

```text
QUERY PRE  players=12 max=32 verified=1
QUERY POST players=1  max=32 verified=1
QUERY NAMES count=12 players=1
```

A contagem estava correta imediatamente depois da interpretação do EYE2 e mudava durante `UpdatePingStatus`.

Isto excluiu a camada de interface e o código de desenho da lista de servidores.

## 4. Remover o PingStatus não era solução

Uma build de teste removeu o bloco PingStatus, mantendo a contagem virtual intacta.

O cliente passou então a indicar:

```text
QUERY PRE  players=12 max=32 verified=1
QUERY POST players=1  max=32 verified=0
```

Isto confirmou que o PingStatus fazia parte do mecanismo de validação/recuperação. A solução correta não era removê-lo.

## 5. Captura de amostras reais de PingStatus

O teste seguinte capturou os bytes originais do PingStatus com diferentes estados de jogadores reais. Os bytes mudavam entre pedidos mesmo quando a quantidade de jogadores reais se mantinha igual.

Isto provou que o bloco não era uma simples tabela fixa do tipo «1 jogador = estes bytes».

## 6. Reutilizar o encoder original

A análise estática do `net.dll` do Windows localizou o encoder interno utilizado pelo caminho `GetPingStatus` do servidor.

Em vez de fazer engenharia inversa ao formato codificado propriamente dito, o módulo nativo chamou o **encoder original** com uma contagem de jogadores diferente:

```text
jogadores reais + jogadores virtuais
```

Depois de substituir o campo PingStatus pelo valor gerado, o cliente de diagnóstico mostrou:

```text
QUERY PRE  players=12 max=32 verified=1
QUERY POST players=12 max=32 verified=1
LIST POST  nPlayers=12 max=32 names=12 verified=1
```

O mesmo cliente MTA original passou depois a mostrar 12/32 sem jogadores reais, 13/32 com um jogador real mais 12 virtuais e 29/32 com um jogador real mais 28 virtuais.

## 7. Port para Linux x64

O servidor de teste com características de produção utilizava Linux x64, onde o módulo de rede é `x64/net.so`, e não `net.dll`.

O port para Linux exigiu:

- uma ABI de encoder diferente para `std::string` da libstdc++;
- localizar e validar o offset do encoder em Linux;
- interceção PLT/GOT de `sendto` em `deathmatch.so`, em vez do patch da IAT do Windows;
- colocação do módulo x64 em `x64/modules/`.

O source público mantém as implementações Windows e Linux separadas porque o loader, a ABI e os mecanismos de hook são diferentes.

## 8. Limpeza para a versão pública

A versão pública removeu deliberadamente branding específico do projeto e a lista interna de nomes original. É distribuída com um `names.lua` vazio e com todo o código-fonte necessário para inspecionar, modificar e recompilar o resource e os módulos nativos.

## Lições

- Um pacote que parece correto na rede pode voltar a ser validado por outro subsistema do cliente.
- Instrumentar o source oficial do cliente foi mais útil do que continuar a adivinhar bytes do pacote.
- Reutilizar o encoder original foi mais seguro e mais sustentável do que tentar copiar um formato opaco a partir de amostras.
- Fingerprints de builds e validações por assinatura são essenciais quando um módulo nativo chama código interno específico de uma versão.
