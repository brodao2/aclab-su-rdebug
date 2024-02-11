import * as vscode from 'vscode';
import { SuDebugConfigurationProvider } from './debug/configurationProvider';
import { SuDebugAdapterDescriptorFactory } from './debug/debugAdapterDescriptorFactory';
import { SuDebugAdapterTrackerFactory } from './debug/debugTrackerDescriptorFactory';

//import { registerInspectorView } from "./inspector";

//const asyncExec = promisify(child_process.exec);

let outputChannel: vscode.OutputChannel;

export function pp(obj: any, obj1?: any) {
	let text: string = JSON.stringify(obj);

	if (obj1) {
		text += "\n" + JSON.stringify(obj1) + "\n";
	}

	outputChannel.appendLine(text);
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
		vscode.debug.registerDebugConfigurationProvider(SU_TYPE_DEBUG, new SuDebugConfigurationProvider()),
		vscode.debug.registerDebugAdapterDescriptorFactory(SU_TYPE_DEBUG, new SuDebugAdapterDescriptorFactory()),
		vscode.debug.registerDebugAdapterTrackerFactory(SU_TYPE_DEBUG, new SuDebugAdapterTrackerFactory())
	);

	// context.subscriptions.push(
	// 	vscode.debug.onDidChangeBreakpoints((event: vscode.BreakpointsChangeEvent) => {
	// 		procesChangeBreakpointsEvent(languageClient, event);
	// 	})
	// );

	context.subscriptions.push(
		vscode.debug.onDidReceiveDebugSessionCustomEvent(
			(debugEvent: vscode.DebugSessionCustomEvent) => {
				console.log("onDidReceiveDebugSessionCustomEvent");
			}
		)
	);

	context.subscriptions.push(
		vscode.debug.onDidStartDebugSession(async session => {
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
		})
	);

	context.subscriptions.push(
		vscode.debug.onDidTerminateDebugSession(() => {
			console.log("onDidTerminateDebugSession");
		})
	);

	context.subscriptions.push(
		vscode.debug.onDidChangeActiveDebugSession(
			(debugEvent: vscode.DebugSession | undefined) => {
				console.log("onDidChangeActiveDebugSession");
			}
		)
	);
}

export function deactivate() {
	//
}