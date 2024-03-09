/*
Copyright 2024 Alan Cândido (brodao@gmail.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http: //www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

const { spawn } = require('child_process');
import * as vscode from "vscode";
import * as path from "path";

export class SketchUpProcess {
    private logger: string;
    private command: string;
    private args: string[];
    private spawn: any;
    private outputChannel: vscode.OutputChannel;

    constructor(command: string, args: string[]) {
        this.logger = "";
        this.command = command;
        this.args = args;

        this.outputChannel = vscode.window.createOutputChannel(path.basename(command));
    }

    launch(remotePort: number): boolean {
        let args: string[] = [
            "-rdebug",
            `ide port=${remotePort}`,
            ...this.args
        ];

        this.spawn = spawn(this.command, args);
        this.spawn.stdout.on('data', (data: any) => {
            console.log(`stdout:\n${data}`);
            this.outputChannel.appendLine(`stdout:\n${data}`);
        });

        this.spawn.stderr.on('data', (data: any) => {
            console.error(`stderr: ${data}`);
            this.outputChannel.appendLine(`stderr:\n${data}`);
        });

        this.spawn.stderr.on("exit", (code: any) => {
            if (code !== 0) {
                vscode.window.showErrorMessage(`Couldn't start debug session. The debugger process exited with code ${code}`);
            }

            return true;
        });

        return true;
    }

    stop() {
        this.spawn.close();

        this.outputChannel.dispose();
        this.outputChannel;
    }

}