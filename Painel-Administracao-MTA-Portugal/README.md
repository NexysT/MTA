<p align="center">
  <img src="../assets/mta-banner.svg" alt="MTA Portugal: recursos para Multi Theft Auto, por NexysT" width="100%">
</p>

<p align="center">
  <a href="../README.md">← Voltar à página dos recursos</a>
  &nbsp;·&nbsp;
  <a href="https://github.com/NexysT">Perfil do NexysT</a>
</p>

<h1 align="center">Painel Administração | MTA Portugal</h1>

<p align="center">
  <img src="https://img.shields.io/badge/MTA-San_Andreas-36d6a6?style=flat-square&labelColor=102722" alt="MTA:SA">
  <img src="https://img.shields.io/badge/IDIOMA-PT--PT-7aabeb?style=flat-square&labelColor=102722" alt="Português de Portugal">
  <img src="https://img.shields.io/badge/SCRIPTS-Lua_compilado-36d6a6?style=flat-square&labelColor=102722" alt="Scripts Lua compilados">
</p>

O `admin` é aquele painel de que precisas quando estás a tomar conta de um servidor MTA: consultar um jogador, expulsar alguém, verificar os banimentos, reiniciar um resource ou alterar uma opção do servidor. O painel original já fazia muita coisa. O problema é que, para o que eu queria no MTA Portugal, faltavam-lhe algumas ferramentas e a interface em inglês já não fazia muito sentido.

Fui mantendo a base do recurso e trabalhando por cima dela: tradução para português de Portugal, ajustes visuais e novas abas para depuração, comandos e permissões de staff. A intenção é conseguires resolver mais coisas a partir do mesmo painel, sem ter de andar constantemente entre a consola, os scripts e a gestão ACL antiga.

**Se só queres instalar, podes ir diretamente para [Como instalar](#como-instalar).** Se queres perceber primeiro o que mudou, continua a ler.

---

## O que encontras no painel?

| Aba | Para que serve |
| :--- | :--- |
| **Jogadores** | Selecionas um jogador para consultar informações e usar as ações a que o teu cargo tem acesso: observar, expulsar, banir, alterar vida, colete, dinheiro, posição e outras opções. |
| **Recursos** | Consultas os resources do servidor e, quando tens permissão, podes iniciá-los, pará-los ou reiniciá-los. |
| **Mapas** | Consultas os mapas e as opções de mudança de mapa disponíveis. |
| **Servidor** | Reúne definições do servidor, como a password de entrada e outras configurações administrativas. |
| **Banimentos** | Consultas os banimentos e geres as ações de banir e desbanir de acordo com as permissões atribuídas. |
| **Depuração** | Vês mensagens de informação, avisos e erros enviados para a aba. Dá jeito quando estás a testar resources e queres perceber rapidamente o que aconteceu. |
| **Comandos** | Escolhes um resource na lista e encontras os comandos que ele registou, em vez de procurar em cada ficheiro Lua. |
| **ACL** | Geres cargos, membros e permissões numa interface mais próxima da lógica dos cargos do Discord. |
| **Chat** | Acesso às mensagens e ferramentas de comunicação administrativa. |
| **Opções** | Preferências do próprio painel e opções relacionadas com a conta. |

### Depuração: para quem anda a desenvolver

Imagina que acabaste de mexer num resource e ele começou a dar um aviso. Em vez de saíres do painel para ir procurar a mensagem, podes abrir **Depuração**, pesquisar pelo texto ou pelo recurso e consultar o que foi recebido.

A aba tem filtros e ferramentas de pesquisa, pausa, limpeza e cópia de mensagens. Mostra o que chega do **servidor** e do **cliente onde tens o painel aberto**. Não é um acesso mágico ao `debugscript 3` local de todos os jogadores: uma mensagem que só acontece no computador de outro jogador não aparece automaticamente no teu.

### Comandos: «como é que abro este sistema?»

Se tens muitos resources, acabas por te esquecer de alguns comandos. A aba **Comandos** foi feita para isso: procuras o resource do lado esquerdo, clicas nele e consultas os comandos registados do lado direito. Ajuda especialmente nos testes, quando tens vários comandos temporários de desenvolvimento.

### ACL: cargos sem decorar centenas de permissões

Na gestão de ACL, podes trabalhar com cargos e membros e escolher permissões de forma mais visual. A ideia é que um Moderador, por exemplo, possa consultar jogadores e executar determinadas ações, mas não precise de ter acesso às definições mais sensíveis do servidor.

**Abrir uma aba não é o mesmo que ter autorização para executar tudo o que está lá dentro.** As permissões continuam a ser verificadas pelo servidor. A conta de quem está a usar o painel tem de pertencer aos grupos certos e ter os direitos necessários.

O gestor ACL tradicional foi mantido para situações em que precises de consultar ou ajustar direitos mais técnicos.

### Admin anónimo: o que aparece e o que fica guardado

O modo **Admin anónimo** serve para reduzir a exposição do nome do staff nas ações administrativas abrangidas. Se estiver ativo e o administrador tiver o direito `command.anonadmin`, as mensagens dessas ações deixam de identificar quem as executou; nas alterações silenciosas, não é mostrada uma mensagem de chat ao jogador.

Isto **não significa que a ação deixa de ser registada**. O histórico administrativo mantém informação interna sobre quem a executou. É importante para se conseguir perceber mais tarde quem alterou uma vida, aplicou uma sanção ou mexeu numa permissão.

O modo aplica-se às mensagens emitidas pelo próprio Admin. Se outro resource independente anunciar alterações por iniciativa própria, terá de ser integrado separadamente.

---

## Como descarregar

Na [página principal do repositório MTA](https://github.com/NexysT/MTA), clica no botão verde **Code** e escolhe **Download ZIP**.

Extrai o ZIP. Vais encontrar uma pasta com este caminho:

```text
MTA-main/
└── Painel-Administracao-MTA-Portugal/
    └── admin/
```

**É só a pasta `admin/` que deves copiar para o teu servidor.** A pasta `Painel-Administracao-MTA-Portugal` existe apenas para organizar o GitHub e guardar esta documentação.

Se já estás a ver a pasta `admin/` diretamente no GitHub, podes também descarregar o repositório e ir buscá-la dentro do ZIP.

## Como instalar

Antes de substituir um painel que já funciona, guarda uma cópia. Isto é particularmente importante se já tens grupos, permissões e configurações próprias no servidor.

1. **Guarda o Admin atual.** Faz uma cópia de `mods/deathmatch/resources/admin/` para outra pasta, fora do diretório dos resources ativos.
2. **Guarda o `acl.xml` do servidor.** Normalmente está em `mods/deathmatch/acl.xml`. Não o substituas por uma configuração de exemplo.
3. **Copia a pasta nova.** Coloca a pasta `admin/` deste repositório em `mods/deathmatch/resources/`, substituindo o resource anterior. Confirma que não ficou `resources/admin/admin/meta.xml`: o `meta.xml` deve estar diretamente dentro de `resources/admin/`.
4. **Atualiza os resources.** Na consola do servidor, executa:

```text
refresh
restart admin
```

Se o resource ainda não estiver em execução, utiliza `start admin` em vez de `restart admin`.

5. **Entra com uma conta administrativa de teste.** Abre o painel (por defeito, tecla **P**), confirma que as abas aparecem e experimenta primeiro uma ação sem impacto nos outros jogadores.
6. **Confirma a ACL.** Se um botão estiver bloqueado ou uma aba não aparecer, verifica a conta MTA, os grupos e os direitos atribuídos. Não tentes resolver tudo dando acesso total a toda a staff.

> O `meta.xml` carrega os scripts com os nomes atuais. Não é preciso renomear os `.lua` para `.luac` nem alterar os caminhos dos ficheiros.

---

## Se alguma coisa não funcionar

<details>
<summary><strong>O painel não abre quando carrego em P</strong></summary>

Verifica se o resource iniciou sem erros, se iniciaste sessão numa conta MTA com acesso administrativo e se essa conta tem o direito `general.adminpanel`. Abre `debugscript 3` e procura a primeira mensagem de erro associada ao `admin`.
</details>

<details>
<summary><strong>Consigo abrir o painel, mas há abas ou botões bloqueados</strong></summary>

Isso costuma ser uma questão de ACL. O painel não dá permissões automaticamente a quem consegue vê-lo. Confirma o grupo da conta e os direitos dessa ação. Na aba ACL, usa uma conta autorizada para gerir os cargos.
</details>

<details>
<summary><strong>A Depuração está vazia</strong></summary>

A aba só apresenta as mensagens que recebe. Confirma que tens permissão para a consultar e provoca um aviso controlado num recurso de teste. Os erros locais de outros jogadores não são recolhidos automaticamente.
</details>

<details>
<summary><strong>Porque é que o GitHub não mostra código Lua quando abro um ficheiro?</strong></summary>

Os scripts publicados foram compilados e ofuscados antes da distribuição. O nome continua a terminar em `.lua`, mas o conteúdo não é o texto original. Isso não é um problema de transferência nem significa que o ficheiro esteja vazio.
</details>

<details>
<summary><strong>Posso trocar o nome da pasta `admin`?</strong></summary>

Não é aconselhável. Outros resources e definições do servidor podem referir-se ao nome `admin`. O nome da página no GitHub pode ser diferente; a pasta instalada deve continuar a chamar-se `admin`.
</details>

---

## O que está dentro da pasta?

```text
Painel-Administracao-MTA-Portugal/
├── README.md                   ← este guia
├── LICENSE-MTA.txt             ← licença da base oficial
└── admin/                      ← copiar esta pasta para o servidor
    ├── meta.xml                ← diz ao MTA o que deve carregar
    ├── admin_definitions.lua
    ├── client/                 ← interface executada no cliente
    ├── server/                 ← operações e validação do servidor
    ├── conf/                   ← configurações do recurso
    └── logs/                   ← ficheiros de registo do recurso
```

Os ficheiros `.lua` deste pacote mantêm a extensão usada no `meta.xml`, embora o conteúdo tenha sido distribuído compilado/ofuscado. Isso torna a leitura direta mais difícil, mas não é uma barreira impossível de ultrapassar.

**Não publiques os teus logs reais, passwords ou outros dados do servidor quando partilhares uma versão modificada.**

---

## Origem, créditos e licença

Este painel **não nasceu do zero**. A base é o recurso `admin` da coleção oficial [mtasa-resources](https://github.com/multitheftauto/mtasa-resources), associado originalmente a **lil_Toady**.

A tradução, as alterações à interface e as funcionalidades acrescentadas nesta adaptação são trabalho desenvolvido para **MTA Portugal (NexysT)**. Os direitos dos autores da base continuam a aplicar-se, independentemente de os scripts estarem compilados.

A coleção oficial é publicada sob licença **MIT**, cujo texto e aviso de atribuição estão reproduzidos em [LICENSE-MTA.txt](./LICENSE-MTA.txt). Essa licença permite utilização, alteração e redistribuição nas condições indicadas no próprio documento.

<p align="center">
  <a href="../README.md"><strong>← Ver os outros recursos do MTA Portugal</strong></a>
</p>
<p align="center"><sub>MTA Portugal · NexysT · Português de Portugal</sub></p>
