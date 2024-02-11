const { spawn } = require('child_process');

export class SketchUpProcess {
    private logger: string;
    private command: string;
    private args: string[];
    spawn: any;

    constructor(logger: string, command: string, args: string[]) {
        this.logger = logger;
        this.command = command;
        this.args = args;
    }

    launcher(): boolean {
        this.spawn = spawn(this.command, this.args);
        this.spawn.stdout.on('data', (data: any) => {
            console.log(`stdout:\n${data}`);
        });

        this.spawn.stderr.on('data', (data: any) => {
            console.error(`stderr: ${data}`);
        });

        return true;
    }
}