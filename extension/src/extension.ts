import * as vscode from 'vscode';
import { SuConfigurationProvider } from "./debug/SuConfigurationProvider";
import { SuAdapterDescriptorFactory } from "./debug/debugAdapterDescriptorFactory";
import { SuDebugAdapterTrackerFactory } from './debug/debugTrackerFactory';

//import { registerInspectorView } from "./inspector";

//const asyncExec = promisify(child_process.exec);

let outputChannel: vscode.OutputChannel;
export function pp(obj: any) {
	outputChannel.appendLine(JSON.stringify(obj));
}


//const outputTerminals = new Map<string, vscode.Terminal>();
// let lastExecCommand: string | undefined;
// let lastProgram: string | undefined;

//const terminalName: string = "Sketch Debug Terminal";

// function workspaceFolder(): string | undefined {
// 	if (vscode.workspace.workspaceFolders) {
// 		for (const ws of vscode.workspace.workspaceFolders) {
// 			return ws.uri.fsPath;
// 		}
// 	}
// }

// function customPath(workingDirectory: string): string {
// 	if (path.isAbsolute(workingDirectory)) {
// 		return workingDirectory;
// 	} else {
// 		const wspath = workspaceFolder();

// 		if (wspath) {
// 			return path.join(wspath, workingDirectory);
// 		} else {
// 			return workingDirectory;
// 		}
// 	}
// }

// function exportBreakpoints() {
// 	if (vscode.workspace.getConfiguration("rdbg").get("saveBreakpoints")) {
// 		const wspath = workspaceFolder();

// 		if (wspath) {
// 			var bpLines = "";
// 			for (const bp of vscode.debug.breakpoints) {
// 				if (bp instanceof vscode.SourceBreakpoint && bp.enabled) {
// 					// outputChannel.appendLine(JSON.stringify(bp));
// 					const startLine = bp.location.range.start.line + 1;
// 					const path = bp.location.uri.path;
// 					bpLines = bpLines + "break " + path + ":" + startLine + "\n";
// 				}
// 			}
// 			const bpPath = path.join(wspath, ".rdbgrc.breakpoints");
// 			fs.writeFile(bpPath, bpLines, () => { });
// 			outputChannel.appendLine("Written: " + bpPath);
// 		}
// 	}
// }

const SU_TYPE_DEBUG: string = "aclab-su-rdebug";

export function activate(context: vscode.ExtensionContext) {
	outputChannel = vscode.window.createOutputChannel("su-debug");

	context.subscriptions.push(
		vscode.debug.registerDebugConfigurationProvider(SU_TYPE_DEBUG, new SuConfigurationProvider()),
		vscode.debug.registerDebugAdapterDescriptorFactory(SU_TYPE_DEBUG, new SuAdapterDescriptorFactory()),
		vscode.debug.registerDebugAdapterTrackerFactory(SU_TYPE_DEBUG, new SuDebugAdapterTrackerFactory())
	);

	// context.subscriptions.push(
	// 	vscode.debug.onDidChangeBreakpoints((event: vscode.BreakpointsChangeEvent) => {
	// 		procesChangeBreakpointsEvent(languageClient, event);
	// 	})
	// );

	// context.subscriptions.push(
	// 	vscode.debug.onDidReceiveDebugSessionCustomEvent(
	// 		(debugEvent: vscode.DebugSessionCustomEvent) => {
	// 			_debugEvent = debugEvent;
	// 			processDebugCustomEvent(debugEvent);
	// 		}
	// 	)
	// );

	context.subscriptions.push(
		vscode.debug.onDidTerminateDebugSession(() => {
			//_debugEvent = undefined;
		})
	);


	// const adapterDescriptorFactory = new SuAdapterDescriptorFactory(context);
	// const DAPTrackQueue = new EventEmitter<any>();
	// context.subscriptions.push(vscode.debug.registerDebugConfigurationProvider("rdbg", new RdbgInitialConfigurationProvider()));
	// context.subscriptions.push(vscode.debug.registerDebugAdapterDescriptorFactory("rdbg", adapterDescriptorFactory));
	// context.subscriptions.push(vscode.debug.registerDebugAdapterTrackerFactory("rdbg", new RdbgDebugAdapterTrackerFactory(DAPTrackQueue)));

	//
	// context.subscriptions.push(vscode.debug.onDidChangeBreakpoints(() => {
	// 	exportBreakpoints();
	// }));

	context.subscriptions.push(vscode.debug.onDidStartDebugSession(async session => {
		const config = session.configuration;
		if (config.request !== "launch" || config.useTerminal || config.noDebug) { return; };

		// const args: DebugProtocol.EvaluateArguments = {
		// 	expression: ",eval $stdout.sync=true",
		// 	context: "repl"
		// };
		// try {
		// 	await session.customRequest("evaluate", args);
		// } catch (err) {
		// 	// We need to ignore the error because this request will be failed if the version of rdbg is older than 1.7. The `,command` API is introduced from version 1.7.
		// 	pp(err);
		// }
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

	// const disp = registerInspectorView(DAPTrackQueue, adapterDescriptorFactory);
	// context.subscriptions.concat(disp);
}

export function deactivate() {
	//
}


// class RdbgInitialConfigurationProvider implements vscode.DebugConfigurationProvider {
// 	resolveDebugConfiguration(_folder: WorkspaceFolder | undefined, config: DebugConfiguration, _token?: CancellationToken): ProviderResult<DebugConfiguration> {
// 		const extensions = [];
// 		const traceEnabled = vscode.workspace.getConfiguration("rdbg").get<boolean>("enableTraceInspector");
// 		if (traceEnabled) {
// 			extensions.push("traceInspector");
// 		}

// 		config.rdbgExtensions = extensions;
// 		config.rdbgInitialScripts = []; // for future extension

// 		if (config.script || config.request === "attach") {
// 			return config;
// 		}

// 		if (Object.keys(config).length > 0 && !config.script) {
// 			return vscode.window.showInformationMessage("Cannot find a program to debug").then(_ => {
// 				return null;
// 			});
// 		}

// 		// launch without configuration
// 		if (vscode.window.activeTextEditor?.document.languageId !== "ruby") {
// 			return vscode.window.showInformationMessage("Select a ruby file to debug").then(_ => {
// 				return null;
// 			});
// 		}
// 		return {
// 			type: "rdbg",
// 			name: "Launch",
// 			request: "launch",
// 			script: "${file}",
// 			askParameters: true,
// 		};
// 	};

// 	// provideDebugConfigurations(_folder: WorkspaceFolder | undefined): ProviderResult<DebugConfiguration[]> {
// 	// 	return [
// 	// 		{
// 	// 			type: "rdbg",
// 	// 			name: "Debug current file with rdbg",
// 	// 			request: "launch",
// 	// 			script: "${file}",
// 	// 			args: [],
// 	// 			askParameters: true,
// 	// 		},
// 	// 		{
// 	// 			type: "rdbg",
// 	// 			name: "Attach with rdbg",
// 	// 			request: "attach",
// 	// 		}
// 	// 	];
// 	// };
// }

// class StopDebugAdapter implements vscode.DebugAdapter {
// 	private sendMessage = new vscode.EventEmitter<any>();
// 	readonly onDidSendMessage: vscode.Event<any> = this.sendMessage.event;

// 	handleMessage(): void {
// 		const ev = {
// 			type: "event",
// 			seq: 1,
// 			event: "terminated",
// 		};
// 		this.sendMessage.fire(ev);
// 	}

// 	dispose() {
// 	}
// }

// const findSuTerminal = (): vscode.Terminal | undefined => {
// 	let terminal: vscode.Terminal | undefined;
// 	const currentTerminals: vscode.Terminal[] = Array.from(outputTerminals.values());
// 	for (const t of vscode.window.terminals) {
// 		if (t.name === terminalName && !t.exitStatus && !currentTerminals.includes(t)) {
// 			terminal = t;
// 			break;
// 		}
// 	}

// 	return terminal;
// };