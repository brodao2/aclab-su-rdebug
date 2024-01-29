import * as child_process from "child_process";
import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";
import { promisify } from "util";

import {
	CancellationToken,
	DebugAdapterDescriptor,
	DebugAdapterDescriptorFactory,
	DebugAdapterExecutable,
	DebugAdapterInlineImplementation,
	DebugAdapterNamedPipeServer,
	DebugConfiguration,
	DebugSession,
	ProviderResult,
	WorkspaceFolder,
	ThemeIcon,
	EventEmitter
} from "vscode";

import { DebugProtocol } from "@vscode/debugprotocol";
import { registerInspectorView } from "./inspector";
import { SuAttachConfiguration, SuLaunchConfiguration } from "./config";

const asyncExec = promisify(child_process.exec);

let outputChannel: vscode.OutputChannel;
const outputTerminals = new Map<string, vscode.Terminal>();
// let lastExecCommand: string | undefined;
// let lastProgram: string | undefined;

const terminalName: string = "Sketch Debug Terminal";

function workspaceFolder(): string | undefined {
	if (vscode.workspace.workspaceFolders) {
		for (const ws of vscode.workspace.workspaceFolders) {
			return ws.uri.fsPath;
		}
	}
}

function customPath(workingDirectory: string): string {
	if (path.isAbsolute(workingDirectory)) {
		return workingDirectory;
	} else {
		const wspath = workspaceFolder();

		if (wspath) {
			return path.join(wspath, workingDirectory);
		} else {
			return workingDirectory;
		}
	}
}

function pp(obj: any) {
	outputChannel.appendLine(JSON.stringify(obj));
}

function exportBreakpoints() {
	if (vscode.workspace.getConfiguration("rdbg").get("saveBreakpoints")) {
		const wspath = workspaceFolder();

		if (wspath) {
			var bpLines = "";
			for (const bp of vscode.debug.breakpoints) {
				if (bp instanceof vscode.SourceBreakpoint && bp.enabled) {
					// outputChannel.appendLine(JSON.stringify(bp));
					const startLine = bp.location.range.start.line + 1;
					const path = bp.location.uri.path;
					bpLines = bpLines + "break " + path + ":" + startLine + "\n";
				}
			}
			const bpPath = path.join(wspath, ".rdbgrc.breakpoints");
			fs.writeFile(bpPath, bpLines, () => { });
			outputChannel.appendLine("Written: " + bpPath);
		}
	}
}

export function activate(context: vscode.ExtensionContext) {
	outputChannel = vscode.window.createOutputChannel("rdbg");

	const adapterDescriptorFactory = new SuAdapterDescriptorFactory(context);
	const DAPTrackQueue = new EventEmitter<any>();
	context.subscriptions.push(vscode.debug.registerDebugConfigurationProvider("rdbg", new RdbgInitialConfigurationProvider()));
	context.subscriptions.push(vscode.debug.registerDebugAdapterDescriptorFactory("rdbg", adapterDescriptorFactory));
	context.subscriptions.push(vscode.debug.registerDebugAdapterTrackerFactory("rdbg", new RdbgDebugAdapterTrackerFactory(DAPTrackQueue)));

	//
	context.subscriptions.push(vscode.debug.onDidChangeBreakpoints(() => {
		exportBreakpoints();
	}));

	context.subscriptions.push(vscode.debug.onDidStartDebugSession(async session => {
		const config = session.configuration;
		if (config.request !== "launch" || config.useTerminal || config.noDebug) { return; };

		const args: DebugProtocol.EvaluateArguments = {
			expression: ",eval $stdout.sync=true",
			context: "repl"
		};
		try {
			await session.customRequest("evaluate", args);
		} catch (err) {
			// We need to ignore the error because this request will be failed if the version of rdbg is older than 1.7. The `,command` API is introduced from version 1.7.
			pp(err);
		}
	}));

	const folders = vscode.workspace.workspaceFolders;

	if (folders !== undefined && folders.length > 0) {
		// const jsonPath = path.join(folders[0].uri.fsPath, ".vscode/rdbg_autoattach.json");
		// if (fs.existsSync(jsonPath)) {
		// 	const c: AttachConfiguration = JSON.parse(fs.readFileSync(jsonPath, "utf8"));

		// 	if (autoAttachConfigP(c)) {
		// 		fs.unlinkSync(jsonPath);
		// 		vscode.debug.startDebugging(folders[0], c);
		// 		return;
		// 	}
		// }
	}

	const disp = registerInspectorView(DAPTrackQueue, adapterDescriptorFactory);
	context.subscriptions.concat(disp);
}

export function deactivate() {
}

class RdbgDebugAdapterTrackerFactory implements vscode.DebugAdapterTrackerFactory {
	constructor(
		public readonly _emitter: vscode.EventEmitter<any>,
	) { }
	createDebugAdapterTracker(session: DebugSession): ProviderResult<vscode.DebugAdapterTracker> {
		const self = this;
		const tracker: vscode.DebugAdapterTracker = {
			onWillStartSession(): void {
				outputChannel.appendLine("[Start session]\n" + JSON.stringify(session));
			},
			onWillStopSession(): void {
				const outputTerminal = outputTerminals.get(session.id);
				if (outputTerminal) {
					outputTerminal.show();
					outputTerminals.delete(session.id);
				}
			},
			onError(e) {
				outputChannel.appendLine("[Error on session]\n" + e.name + ": " + e.message + "\ne: " + JSON.stringify(e));
			}
		};
		if (session.configuration.showProtocolLog) {
			tracker.onDidSendMessage = (message: any): void => {
				self.publishMessage(message);
				outputChannel.appendLine("[DA->VSCode] " + JSON.stringify(message));
			};
			tracker.onWillReceiveMessage = (message: any): void => {
				outputChannel.appendLine("[VSCode->DA] " + JSON.stringify(message));
			};
		} else {
			tracker.onDidSendMessage = (message: any): void => {
				self.publishMessage(message);
			};
		}
		return tracker;
	}

	private publishMessage(message: any) {
		this._emitter.fire(message);
	}
}

class RdbgInitialConfigurationProvider implements vscode.DebugConfigurationProvider {
	resolveDebugConfiguration(_folder: WorkspaceFolder | undefined, config: DebugConfiguration, _token?: CancellationToken): ProviderResult<DebugConfiguration> {
		const extensions = [];
		const traceEnabled = vscode.workspace.getConfiguration("rdbg").get<boolean>("enableTraceInspector");
		if (traceEnabled) {
			extensions.push("traceInspector");
		}

		config.rdbgExtensions = extensions;
		config.rdbgInitialScripts = []; // for future extension

		if (config.script || config.request === "attach") {
			return config;
		}

		if (Object.keys(config).length > 0 && !config.script) {
			return vscode.window.showInformationMessage("Cannot find a program to debug").then(_ => {
				return null;
			});
		}

		// launch without configuration
		if (vscode.window.activeTextEditor?.document.languageId !== "ruby") {
			return vscode.window.showInformationMessage("Select a ruby file to debug").then(_ => {
				return null;
			});
		}
		return {
			type: "rdbg",
			name: "Launch",
			request: "launch",
			script: "${file}",
			askParameters: true,
		};
	};

	// provideDebugConfigurations(_folder: WorkspaceFolder | undefined): ProviderResult<DebugConfiguration[]> {
	// 	return [
	// 		{
	// 			type: "rdbg",
	// 			name: "Debug current file with rdbg",
	// 			request: "launch",
	// 			script: "${file}",
	// 			args: [],
	// 			askParameters: true,
	// 		},
	// 		{
	// 			type: "rdbg",
	// 			name: "Attach with rdbg",
	// 			request: "attach",
	// 		}
	// 	];
	// };
}

class StopDebugAdapter implements vscode.DebugAdapter {
	private sendMessage = new vscode.EventEmitter<any>();
	readonly onDidSendMessage: vscode.Event<any> = this.sendMessage.event;

	handleMessage(): void {
		const ev = {
			type: "event",
			seq: 1,
			event: "terminated",
		};
		this.sendMessage.fire(ev);
	}

	dispose() {
	}
}

const findSuTerminal = (): vscode.Terminal | undefined => {
	let terminal: vscode.Terminal | undefined;
	const currentTerminals: vscode.Terminal[] = Array.from(outputTerminals.values());
	for (const t of vscode.window.terminals) {
		if (t.name === terminalName && !t.exitStatus && !currentTerminals.includes(t)) {
			terminal = t;
			break;
		}
	}

	return terminal;
};

class SuAdapterDescriptorFactory implements DebugAdapterDescriptorFactory { //, VersionChecker
	private context: vscode.ExtensionContext;
	private sketchUpActivated: boolean;

	constructor(context: vscode.ExtensionContext) {
		this.context = context;
		this.sketchUpActivated = false;
	}

	async createDebugAdapterDescriptor(session: DebugSession, _executable: DebugAdapterExecutable | undefined): Promise<DebugAdapterDescriptor> {
		// session.configuration.internalConsoleOptions = "neverOpen"; // TODO: doesn't affect...
		const c: vscode.DebugConfiguration = session.configuration;
		const cwd = c.cwd ? customPath(c.cwd) : workspaceFolder();

		if (c.type === "launch") {
			const configuration: SuLaunchConfiguration = c as SuLaunchConfiguration;
		} else {
		}

		vscode.commands.executeCommand("rdbg.inspector.startDebugSession", session);

		if (c.request === "attach") {
			return this.attach(session);
		}

		return this.launchOnConsole(session);
	}

	showError(msg: string): void {
		outputChannel.appendLine("Error: " + msg);
		outputChannel.appendLine("Make sure to install rdbg command (`gem install debug`).\n" +
			"If you are using bundler, write `gem 'debug'` in your Gemfile.");
		outputChannel.show();
	}

	// supportLogin(shell: string | undefined) {
	// 	if (shell && (shell.endsWith("bash") || shell.endsWith("zsh") || shell.endsWith("fish"))) {
	// 		return true;
	// 	}
	// 	else {
	// 		return false;
	// 	}
	// }

	// private needShell(shell: string | undefined): boolean {
	// 	return !this.rubyActivated && this.supportLogin(shell)
	// }

	// makeShellCommand(cmd: string) {
	// 	const shell = process.env.SHELL;
	// 	if (this.needShell(shell)) {
	// 		return shell + " -lic '" + cmd + "'";
	// 	} else {
	// 		return cmd;
	// 	}
	// }

	// async getSockList(config: SuAttachConfiguration): Promise<string[]> {
	// 	return new Promise((resolve, reject) => {
	// 		const socks: Array<string> = [];
	// 		socks.push(config.remotePort.toString());
	// 		resolve(socks);
	// 	});
	// }

	// parsePort(port: string): [string | undefined, number | undefined, string | undefined] {
	// 	var m;

	// 	if (port.match(/^\d+$/)) {
	// 		return ["localhost", parseInt(port), undefined];
	// 	}
	// 	else if ((m = port.match(/^(.+):(\d+)$/))) {
	// 		return [m[1], parseInt(m[2]), undefined];
	// 	}
	// 	else {
	// 		return [undefined, undefined, port];
	// 	}
	// }

	// simplifySockList(list: string[]): string[] {
	// 	return list.map(sockPath => {
	// 		return path.basename(sockPath);
	// 	});
	// }

	async attach(session: DebugSession): Promise<DebugAdapterDescriptor> {
		const config = session.configuration as SuAttachConfiguration;
		let port: number = config.remotePort;
		let host: string = "localhost";

		if (config.noDebug) {
			vscode.window.showErrorMessage("Can not attach \"Without debugging\".");
			return new DebugAdapterInlineImplementation(new StopDebugAdapter);
		}

		if (port) {
			return new vscode.DebugAdapterServer(port, host);
		}
		else {
			vscode.window.showErrorMessage("Unreachable.");
			return new DebugAdapterInlineImplementation(new StopDebugAdapter);
		}
	}

	sketchUpBin(config: SuLaunchConfiguration) {

		return config.sketchUpProgram;
	}

	// getSockPath(config: LaunchConfiguration): Promise<string | undefined> {
	// 	return new Promise((resolve) => {
	// 		const command = this.makeShellCommand(this.rdbgBin(config) + " --util=gen-sockpath");
	// 		const p = child_process.exec(command, {
	// 			cwd: config.cwd ? customPath(config.cwd) : workspaceFolder(),
	// 			env: { ...process.env, ...config.env }
	// 		});
	// 		let path: string;

	// 		p.on("error", e => {
	// 			this.showError(e.message);
	// 			resolve(undefined);
	// 		});
	// 		p.on("exit", (code) => {
	// 			if (code !== 0) {
	// 				this.showError("exit code is " + code);
	// 				resolve(undefined);
	// 			}
	// 			else {
	// 				resolve(path);
	// 			}
	// 		});
	// 		p.stderr?.on("data", err => {
	// 			outputChannel.appendLine(err);
	// 		});
	// 		p.stdout?.on("data", out => {
	// 			path = out.trim();
	// 		});
	// 	});
	// }

	// getTcpPortFile(config: LaunchConfiguration): Promise<string | undefined> {
	// 	return new Promise((resolve) => {
	// 		const command = this.makeShellCommand(this.rdbgBin(config) + " --util=gen-portpath");
	// 		const p = child_process.exec(command, {
	// 			cwd: config.cwd ? customPath(config.cwd) : workspaceFolder(),
	// 			env: { ...process.env, ...config.env }
	// 		});
	// 		let path: string;

	// 		p.on("error", () => {
	// 			resolve(undefined);
	// 		});
	// 		p.on("exit", () => {
	// 			resolve(path);
	// 		});
	// 		p.stderr?.on("data", err => {
	// 			outputChannel.appendLine(err);
	// 		});
	// 		p.stdout?.on("data", out => {
	// 			path = out.trim();
	// 		});
	// 	});
	// }

	// getVersion(config: LaunchConfiguration): Promise<string | null> {
	// 	return new Promise((resolve) => {
	// 		const command = this.makeShellCommand(this.rdbgBin(config) + " --version");
	// 		const p = child_process.exec(command, {
	// 			cwd: config.cwd ? customPath(config.cwd) : workspaceFolder(),
	// 			env: { ...process.env, ...config.env }
	// 		});
	// 		let version: string;

	// 		p.on("error", e => {
	// 			this.showError(e.message);
	// 			resolve(null);
	// 		});
	// 		p.on("exit", (code) => {
	// 			if (code !== 0) {
	// 				this.showError(command + ": exit code is " + code);
	// 				resolve(null);
	// 			}
	// 			else {
	// 				resolve(version);
	// 			}
	// 		});
	// 		p.stderr?.on("data", err => {
	// 			outputChannel.appendLine(err);
	// 		});
	// 		p.stdout?.on("data", out => {
	// 			version = out.trim();
	// 		});
	// 	});

	// }

	// vernum(version: string): number {
	// 	const vers = /rdbg (\d+)\.(\d+)\.(\d+)/.exec(version);
	// 	if (vers) {
	// 		return Number(vers[1]) * 1000 * 1000 + Number(vers[2]) * 1000 + Number(vers[3]);
	// 	}
	// 	else {
	// 		return 0;
	// 	}
	// }

	// envPrefix(env?: { [key: string]: string }): string {
	// 	if (env) {
	// 		let prefix = "";
	// 		if (process.platform === "win32") {
	// 			for (const key in env) {
	// 				prefix += "$Env:" + key + "='" + env[key] + "'; ";
	// 			}
	// 		} else {
	// 			for (const key in env) {
	// 				prefix += key + "='" + env[key] + "' ";
	// 			}
	// 		}
	// 		return prefix;
	// 	}
	// 	else {
	// 		return "";
	// 	}
	// }

	sleepMs(waitMs: number) {
		return new Promise((resolve) => setTimeout(resolve, waitMs));
	}

	async waitTcpPortFile(path: string, waitMs: number | undefined) {
		return this.waitUntil(() => {
			return fs.existsSync(path) && fs.readFileSync(path).toString().length > 0;
		}, waitMs);
	}

	// async waitFile(path: string, waitMs: number | undefined): Promise<boolean> {
	// 	return this.waitUntil(() => {
	// 		return fs.existsSync(path);
	// 	}, waitMs);
	// }

	async waitUntil(condition: () => boolean, waitMs: number | undefined) {
		let iterations: number = 50;
		if (waitMs) {
			iterations = waitMs / 100;
		}

		const startTime = Date.now();
		let i = 0;
		while (true) {
			i++;
			if (i > iterations) {
				vscode.window.showErrorMessage("Couldn't start debug session (wait for " + (Date.now() - startTime) + " ms). Please install debug.gem.");
				return false;
			}
			if (condition()) {
				return true;
			}
			await this.sleepMs(100);
		}
	}

	// On Windows, generating `getTcpPortFile` method is always failed if the version of the debug.gem is less than 1.7.1.
	// `invalidRdbgVersion` method checks the version of the debug.gem.
	// FYI: https://github.com/ruby/debug/pull/937
	// async invalidRdbgVersion(config: LaunchConfiguration) {
	// 	const version = await this.getVersion(config);
	// 	if (version === null) {
	// 		return false;
	// 	}
	// 	const vernum = this.vernum(version);
	// 	return vernum < 1007002
	// }

	async launchOnTerminal(session: DebugSession): Promise<DebugAdapterDescriptor> {
		const config = session.configuration as SuLaunchConfiguration;

		// outputChannel.appendLine(JSON.stringify(session));

		// setup debugPort
		let tcpHost: string = "localhost";
		let tcpPort: number = config.remotePort;

		// setup terminal
		let outputTerminal = findSuTerminal();

		if (!outputTerminal) {
			outputTerminal = vscode.window.createTerminal({
				name: terminalName,
				// shellPath: shell,
				// shellArgs: shellArgs,
				message: `Created by aclab-su-rdebug at ${new Date()}`,
				iconPath: new ThemeIcon("ruby")
			});
		}

		outputTerminals.set(session.id, outputTerminal);

		// let execCommand = "";
		// try {
		// 	execCommand = await this._getExecCommands(config);
		// } catch (error) {
		// 	if (error instanceof InvalidExecCommandError) {
		// 		return new DebugAdapterInlineImplementation(new StopDebugAdapter);
		// 	}
		// 	throw error;
		// }

		if (outputTerminal) {
			outputTerminal.show(false);

			// if (config.cwd) {
			// 	// Ensure we are in the requested working directory
			// 	const cdCommand = "cd " + customPath(config.cwd);
			// 	outputTerminal.sendText(cdCommand);
			// }

			// outputTerminal.sendText(cmdline);
		}

		if (config.noDebug) {
			return new DebugAdapterInlineImplementation(new StopDebugAdapter);
		}

		// use NamedPipe
		if (tcpPort !== undefined) {
			const waitMs = config.waitLaunchTime ? config.waitLaunchTime : 5000 /* 5 sec */;
			await this.sleepMs(waitMs);
		}

		return new vscode.DebugAdapterServer(tcpPort, tcpHost);

		// failed
		return new DebugAdapterInlineImplementation(new StopDebugAdapter);
	}

	getExecCommands(config: SuLaunchConfiguration): [cmd: string, args: string[]] {
		const execArgs: string[] = [
			"ide",
			config.remotePort ? `'port=${config.remotePort}"` : "",
			...config.sketchUpArguments
		];

		const execCommand: string = config.sketchUpProgram + " " + execArgs.filter((arg: string) => arg.length > 0).join(" ");

		return [
			config.sketchUpProgram,
			execArgs
		];
	}

	// getTCPRdbgArgs(execCommand: string, host: string, port: number, portPath?: string) {
	// 	const rdbgArgs: string[] = [];
	// 	rdbgArgs.push("--command", "--open", "--stop-at-load");
	// 	rdbgArgs.push("--host=" + host);
	// 	let portArg = port.toString();
	// 	if (portPath) {
	// 		portArg += ":" + portPath;
	// 	}
	// 	rdbgArgs.push("--port=" + portArg);
	// 	rdbgArgs.push("--");
	// 	rdbgArgs.push(...execCommand.trim().split(" "));
	// 	return rdbgArgs;
	// }

	// getUnixRdbgArgs(execCommand: string, sockPath?: string) {
	// 	const rdbgArgs: string[] = [];
	// 	rdbgArgs.push("--command", "--open", "--stop-at-load");
	// 	if (sockPath) {
	// 		rdbgArgs.push("--sock-path=" + sockPath);
	// 	}
	// 	rdbgArgs.push("--");
	// 	rdbgArgs.push(...execCommand.trim().split(" "));
	// 	return rdbgArgs;
	// }

	async launchOnConsole(session: DebugSession): Promise<DebugAdapterDescriptor> {
		const config = session.configuration as SuLaunchConfiguration;
		const debugConsole = vscode.debug.activeDebugConsole;

		// outputChannel.appendLine(JSON.stringify(session));

		const options: child_process.SpawnOptionsWithoutStdio = {
			env: { ...process.env, ...config.env },
			cwd: customPath(config.cwd || ""),
		};

		let tcpHost: string = "localhost";
		let tcpPort: number = config.remotePort;

		if (tcpHost !== undefined && tcpPort !== undefined) {
			try {
				let cmd: string = "";
				let args: string[] = [];

				try {
					[cmd, args] = this.getExecCommands(config);
				} catch (error) {
					if (error instanceof InvalidExecCommandError) {
						return new DebugAdapterInlineImplementation(new StopDebugAdapter);
					}
					throw error;
				}

				[, tcpPort] = await this.runDebuggeeWithTCP(debugConsole, cmd, args, options);
			} catch (error: any) {
				vscode.window.showErrorMessage(error.message);
				return new DebugAdapterInlineImplementation(new StopDebugAdapter);
			}
			return new vscode.DebugAdapterServer(tcpPort, tcpHost);
		}
		// const rdbgArgs = this.getUnixRdbgArgs(execCommand, sockPath);
		// try {
		// 	sockPath = await this.runDebuggeeWithUnix(debugConsole, this.sketchUpBin(config), rdbgArgs, options);
		// } catch (error: any) {
		// 	vscode.window.showErrorMessage(error.message);
		// 	return new DebugAdapterInlineImplementation(new StopDebugAdapter);
		// }
		// if (await this.waitFile(sockPath, config.waitLaunchTime)) {
		// 	return new DebugAdapterNamedPipeServer(sockPath);
		// }
		// failed
		return new DebugAdapterInlineImplementation(new StopDebugAdapter);
	}

	// private colorMessage(message: string, colorCode: number) {
	// 	return `\u001b[${colorCode}m${message}\u001b[0m`;
	// }

	// private readonly unixDomainRegex = /DEBUGGER:\sDebugger\scan\sattach\svia\s.+\((.+)\)/;
	// private readonly colors = {
	// 	red: 31,
	// 	blue: 34
	// };

	// private getSpawnCommand(rdbg: string): string {
	// 	const shell = process.env.SHELL;
	// 	if (shell && this.needShell(shell)) {
	// 		return shell;
	// 	}
	// 	return rdbg;
	// }

	// private getSpawnArgs(rdbg: string, args: string[]): string[] {
	// 	const shell = process.env.SHELL;
	// 	if (this.needShell(shell)) {
	// 		return ['-lic', rdbg + ' ' + args.join(' ')];
	// 	}
	// 	return args;
	// }

	// private runDebuggeeWithUnix(debugConsole: vscode.DebugConsole, rdbg: string, rdbgArgs: string[], options: child_process.SpawnOptionsWithoutStdio) {
	// 	const cmd = this.getSpawnCommand(rdbg);
	// 	const args = this.getSpawnArgs(rdbg, rdbgArgs);
	// 	pp(`Running: ${cmd} ${args?.join(" ")}`);
	// 	let connectionReady = false;
	// 	let sockPath = "";
	// 	let stderr = "";
	// 	return new Promise<string>((resolve, reject) => {
	// 		const debugProcess = child_process.spawn(cmd, args, options);
	// 		debugProcess.stderr.on("data", (chunk) => {
	// 			const msg: string = chunk.toString();
	// 			stderr += msg;
	// 			if (stderr.includes("Error")) {
	// 				reject(new Error(stderr));
	// 			}
	// 			if (stderr.includes("DEBUGGER: wait for debugger connection...")) {
	// 				connectionReady = true;
	// 			}
	// 			const found = stderr.match(this.unixDomainRegex);
	// 			if (found !== null && found.length === 2) {
	// 				sockPath = found[1];
	// 			}
	// 			debugConsole.append(this.colorMessage(msg, this.colors.red));

	// 			if (sockPath.length > 0 && connectionReady) {
	// 				resolve(sockPath);
	// 			}
	// 		});
	// 		debugProcess.stdout.on("data", (chunk) => {
	// 			debugConsole.append(this.colorMessage(chunk.toString(), this.colors.blue));
	// 		});
	// 		debugProcess.on("error", (err) => {
	// 			debugConsole.append(err.message);
	// 			reject(err);
	// 		});
	// 		debugProcess.on("exit", (code) => {
	// 			reject(new Error(`Couldn't start debug session. The debuggee process exited with code ${code}`));
	// 		});
	// 	});
	// }

	// private readonly TCPRegex = /DEBUGGER:\sDebugger\scan\sattach\svia\s.+\((.+):(\d+)\)/;

	private runDebuggeeWithTCP(debugConsole: vscode.DebugConsole, cmd: string, args: string[], options: child_process.SpawnOptionsWithoutStdio) {
		pp(`Running: ${cmd} ${args?.join(" ")}`);

		let connectionReady = false;
		let host = "";
		let port = -1;
		let stderr = "";
		return new Promise<[string, number]>((resolve, reject) => {
			const debugProcess = child_process.spawn(cmd, args, options);
			debugProcess.stderr.on("data", (chunk) => {
				const msg: string = chunk.toString();
				stderr += msg;
				if (stderr.includes("Error")) {
					reject(new Error(stderr));
				}
				if (stderr.includes("DEBUGGER: wait for debugger connection...")) {
					connectionReady = true;
				}
				// const found = stderr.match(this.TCPRegex);
				// if (found !== null && found.length === 3) {
				// 	host = found[1];
				// 	port = parseInt(found[2]);
				// }
				// debugConsole.append(this.colorMessage(msg, this.colors.red));

				if (host.length > 0 && port !== -1 && connectionReady) {
					resolve([host, port]);
				}
			});
			debugProcess.stdout.on("data", (chunk) => {
				//debugConsole.append(this.colorMessage(chunk.toString(), this.colors.blue));
				debugConsole.append(chunk.toString());
			});
			debugProcess.on("error", (err) => {
				debugConsole.append(err.message);
				reject(err);
			});
			debugProcess.on("exit", (code) => {
				reject(new Error(`Couldn't start debug session. The debuggee process exited with code ${code}`));
			});
		});
	}
}

class InvalidExecCommandError extends Error { }
