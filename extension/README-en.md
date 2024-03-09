# AC Lab SketchUp Extension Debugger [(português)](README.md)

> Sorry: automatic Portuguese-English translation.

<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<!--[![Build Status](https://travis-ci.org/brodao2/aclab-su-rdebug.svg?branch=master)](https://travis-ci.org/brodao2/aclab-su-rdebug)-->
![GitHub](https://img.shields.io/github/license/brodao2/aclab-su-rdebug)
<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-1-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->
<!-- markdownlint-enabled -->
<!-- prettier-ignore-end -->

## Features

Allows debugging of [**SketchUp**](https://www.sketchup.com) extensions using the _ruby-debug-ide_ protocol under _DAP_ protocol.

## Requisitos

- Fundamentals of using **VS-Code**.
- [SketchUp Ruby API Debugger](https://github.com/SketchUp/sketchup-ruby-debugger) updated and installed, depending on your operating system.
  Ignore this configuration in **VS-CODE**, performing our [configuration](#configuration).
- [VS Code Shopify Ruby (_shopify.ruby-extensions-pack_)](https://github.com/Shopify/vscode-shopify-ruby) installed and working.
- This extension installed.

## Debug Configuration

### To Add

- Activate the command panel (`F1` or `ctrl+p`)
- Select the command "_Debug: Add Configuration..._"(`debug.addConfiguration`)
- The `launch.json` file will open and present you with a list to choose the debugger. Select `SU: Attach to SketchUp already running` or `SU: Launch SketchUp and attach then`.
- Complete the configuration according to the chosen type.

### launch Request

Use this type to start running **SketchUp** and then the debug process will attach itself to this instance.

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
| Attribute | Description |
| -------- | --------- |
| `remotePort` | Debugger connection port **sketchup-ruby-debugger** |
| `sketchUpProgram` | Full path of the **SketchUp** executable. |
| | You can pin a version or a list of versions. Examples: |
| | Fixed: `C:\\Program Files\\SketchUp\\SketchUp 2023\\SketchUp.exe` |
| | Version 2023/22/21: `C:\\Program Files\\SketchUp\\SketchUp ${1\|2023,2022,2021}\\SketchUp.exe` |
| `sketchUpArguments` | List of arguments to be passed to the **SketchUp** application. Optional, default value being `[]`. |
| `remoteWorkspaceRoot` | Root folder of the **SketchUp** extension to debug. Optional, the default value being `${workspaceFolder}`. |

### Attach

Use this type to attach the debug process to an already running **SketchUp** instance.

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
| Attribute | Description |
| -------- | --------- |
| `remotePort` | Debugger connection port **sketchup-ruby-debugger** |
| `remoteWorkspaceRoot` | Root folder of the **SketchUp** extension to debug. Optional, the default value being `${workspaceFolder}`. |

## Extension Settings

* `aclab-su-rdebug.enable`: Enables/disables this extension.
  Possible values ​​are:
  - `true`: enables.
  - `false`: disables.
* `aclab-su-rdebug.trace`: Allows you to track communication between **VS-Code** and the debug adapter.
  This configuration is useful for debugging, with possible values ​​being:
  - `off`: off and is the default value.
  - `messages`: Displays the message (sent/received).
  - `verbose`: Displays the message (sent/received) in detail.
* `aclab-su-rdebug.trace`: Stores communication in the specified file.

## Known occurrences

No known occurrences.

## Maintainer and Collaborators

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tbody>
    <tr>
      <td align="center" valign="top" width="16.66%"><a href="https://github.com/brodao2"><img src="https://avatars.githubusercontent.com/u/114854608?v=4?s=50" width="50px;" alt="Alan Cândido"/><br /><sub><b>Alan Cândido (maintainer)</b></sub></a><br /><a href="https://github.com/brodao2/aclab-su-rdebug/commits?author=brodao2" title="Code">💻</a> <a href="https://github.com/brodao2/aclab-su-rdebug/commits?author=brodao2" title="Documentation">📖</a></td>
    </tr>
  </tbody>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->
