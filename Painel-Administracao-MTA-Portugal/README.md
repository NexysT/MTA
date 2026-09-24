<p align="center"><img src="../assets/mta-banner.svg" alt="MTA Portugal — painel administrativo por NexysT" width="100%"></p>

<p align="center">
  <a href="../README.md">← Voltar aos recursos</a>
  &nbsp;·&nbsp;
  <a href="https://github.com/NexysT">NexysT</a>
</p>

<h1 align="center">Painel Administração | MTA Portugal</h1>
<p align="center">
  Painel de administração para <strong>Multi Theft Auto: San Andreas</strong>, adaptado para o projeto <strong>MTA Portugal</strong> por <strong>NexysT</strong>.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/MTA-SA-36d6a6?style=flat-square&labelColor=102722" alt="MTA:SA">
  <img src="https://img.shields.io/badge/IDIOMA-PT--PT-7aabeb?style=flat-square&labelColor=102722" alt="Português europeu">
  <img src="https://img.shields.io/badge/SCRIPTS-LuaC-36d6a6?style=flat-square&labelColor=102722" alt="Lua compilado">
</p>

<br>

## ✦ Sobre o painel

Esta versão parte do recurso `admin` original do MTA e integra adaptações gráficas e funcionais para facilitar a gestão do servidor. A estrutura da pasta do recurso conserva o nome técnico `admin`, necessário para uma substituição simples em instalações existentes.

## ✦ Funcionalidades

| Área | Funcionalidades |
| :--- | :--- |
| **Jogadores** | Consulta de informações e aplicação de ações administrativas conforme as permissões. |
| **Recursos e mapas** | Ferramentas de gestão integradas no painel. |
| **Servidor e banimentos** | Operações administrativas e consulta de sanções. |
| **Depuração** | Consulta de mensagens e avisos de depuração disponibilizados ao painel. |
| **Comandos** | Consulta dos comandos registados por resource. |
| **ACL** | Interface de gestão de cargos, membros e permissões. |
| **Chat** | Ferramentas de comunicação administrativa. |
| **Admin anónimo** | Ocultação de mensagens identificadoras nas ações abrangidas, mantendo registos internos de auditoria. |

<br>

## ✦ Estrutura do recurso

A publicação dos ficheiros do jogo fica dentro de `admin/`:

```text
Painel-Administracao-MTA-Portugal/
├── README.md
├── LICENSE-MTA.txt
└── admin/
    ├── meta.xml
    ├── admin_definitions.lua
    ├── client/
    ├── server/
    ├── conf/
    └── ...
```

## ✦ Instalação

1. Fazer uma cópia de segurança do resource `admin` atual e do ficheiro `acl.xml` do servidor.
2. Copiar **apenas a pasta `admin/`** para `mods/deathmatch/resources/`.
3. Na consola do servidor, executar:

```text
refresh
restart admin
```

4. Verificar os avisos no `debugscript 3` e testar as permissões com uma conta de teste.

> **Nota:** os scripts Lua do pacote de distribuição foram fornecidos compilados. Isso não garante proteção absoluta contra engenharia inversa. A interface e as operações administrativas precisam de validação no próprio servidor.

## ✦ Créditos e licença

- **Recurso de origem:** painel `admin` de Multi Theft Auto, originalmente criado por **lil_Toady**.
- **Base oficial:** [multitheftauto/mtasa-resources](https://github.com/multitheftauto/mtasa-resources/tree/master/%5Badmin%5D/admin).
- **Adaptação e extensões:** **MTA Portugal · NexysT**.

O código do projeto oficial `mtasa-resources` está disponibilizado sob a licença MIT, salvo indicação em contrário. O texto de licença e os respetivos créditos estão incluídos em [LICENSE-MTA.txt](./LICENSE-MTA.txt). A compilação dos ficheiros não elimina os direitos e avisos da origem.

<p align="center"><sub>MTA Portugal · NexysT · PT-PT</sub></p>
