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

        return true;
    }

    stop() {
        this.spawn.close();

        this.outputChannel.dispose();
        this.outputChannel;
    }

}