# AC Lab SketchUp Extension Debugger [(english)](README-en.md)

<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<!--[![Build Status](https://travis-ci.org/brodao2/aclab-su-rdebug.svg?branch=master)](https://travis-ci.org/brodao2/aclab-su-rdebug)-->
![GitHub](https://img.shields.io/github/license/brodao2/aclab-su-rdebug)
<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-1-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->
<!-- markdownlint-enabled -->
<!-- prettier-ignore-end -->

## Recursos

Permite a depuração de extensões [**SketchUp**](https://www.sketchup.com) utilizando o protocolo _ruby-debug-ide_ sob protocolo _DAP_.

## Requisitos

- Fundamentos de uso do **VS-Code**.
- [SketchUp Ruby API Debugger](https://github.com/SketchUp/sketchup-ruby-debugger) atualizado e instalado, conforme seu sistema operacional.
  Ignore a configuração deste no **VS-CODE**, efetuando a nossa [configuração](#configuração).
- [VS Code Shopify Ruby (_shopify.ruby-extensions-pack_)](https://github.com/Shopify/vscode-shopify-ruby) instalado e funcional.
- Esta extensão instalada.

## Configuração de depuração

### Adicionar

- Acione o painel de comandos (`F1` ou `ctrl+p`)
- Selecione o comando "_Debug: Add Configuration..._"(`debug.addConfiguration`)
- O arquivo `launch.json` será aberto e lhe apresentará um lista pra escolher o depurador. Selecione `SU: Attach to SketchUp already running` ou `SU: Launch SketchUp and attach then`.
- Finalize a configuração conforme o tipo escolhido. 

### Executar (_launch request_)

Use esse tipo para iniciar a execução do **SketchUp** e na sequência, o processo de depuração se anexará a essa instância.

```code
  ...
  "configurations": [
    {
      "type": "aclab-su-rdebug",
      "request": "launch",
      "name": "Launch SketchUp and attach",
      "sketchUpProgram": "C:\\Program Files\\SketchUp\\SketchUp ${1|2023,2022,2021,2020,2019}\\SketchUp.exe",
      "remotePort": 1234,
      "sketchUpArguments": [
        "\\\sample\\.skp"
      "remoteWorkspaceRoot": "${workspaceFolder}"
    },
  ...
  ]
  ...
```
| Atributo | Descrição |
| -------- | --------- |
| `remotePort` | Porta de conexão do depurador **sketchup-ruby-debugger** |
| `sketchUpProgram` | Caminho completo do executável **SketchUp**. |
|                   | Você pode fixar uma versão ou uma lista de versões. Exemplos: |
| | Fixa: `C:\\Program Files\\SketchUp\\SketchUp 2023\\SketchUp.exe` |
| | Versão 2023/22/21: `C:\\Program Files\\SketchUp\\SketchUp ${1\|2023,2022,2021}\\SketchUp.exe` |
| `sketchUpArguments` | Lista de argumentos a serem repassados a aplicação **SketchUp**. Opcional, sendo valor padrão `[]`. |
| `remoteWorkspaceRoot` | Pasta raiz da extensão **SketchUp** a depurar. Opcional, sendo o valor padrão `${workspaceFolder}`. |

### Anexar (_attach_)

Use esse tipo para anexar o processo de depuração a uma instância do **SketchUp** já em execução.

```code
  ...
  "configurations": [
    {
      "type": "aclab-su-rdebug",
      "request": "attach",
      "name": "Attach to SketchUp",
      "remotePort": 1234,
      "remoteWorkspaceRoot": "${workspaceFolder}"
    },
  ...
  ]
  ...
```
| Atributo | Descrição |
| -------- | --------- |
| `remotePort` | Porta de conexão do depurador **sketchup-ruby-debugger** |
| `remoteWorkspaceRoot` | Pasta raiz da extensão **SketchUp** a depurar. Opcional, sendo o valor padrão `${workspaceFolder}`. |

## Configurações de extensão

* `aclab-su-rdebug.enable`: Habilita/desabilita esta extensão.
  Os valores possíveis são:
  - `true`: habilita.
  - `false`: desabilita.
* `aclab-su-rdebug.trace`: Permite acompanhar a comunicação entre o **VS-Code** e o adaptador de depuração.
  Esta configuração é útil para depuração, sendo os valores possíveis:
  - `off`: desligado e é o valor padrão.
  - `messages`: Apresenta a mensagem (enviada/recebida).
  - `verbose`: Apresenta a mensagem (enviada/recebida) em detalhes.
* `aclab-su-rdebug.trace`: Armazena a comunicação no arquivo especificado.

## Ocorrências conhecidas

Nenhuma ocorrência conhecida.

## Mantenedor e Colaboradores

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tbody>
    <tr>
      <td align="center" valign="top" width="16.66%"><a href="https://github.com/brodao2"><img src="https://avatars.githubusercontent.com/u/114854608?v=4?s=50" width="50px;" alt="Alan Cândido"/><br /><sub><b>Alan Cândido (mantenedor)</b></sub></a><br /><a href="https://github.com/brodao2/aclab-su-rdebug/commits?author=brodao2" title="Code">💻</a> <a href="https://github.com/brodao2/aclab-su-rdebug/commits?author=brodao2" title="Documentation">📖</a></td>
    </tr>
  </tbody>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->
