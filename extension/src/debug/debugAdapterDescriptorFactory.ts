import * as vscode from "vscode";
import * as path from "path";
import { SuDebugConfiguration } from "./debugConfigs";
import { SketchUpProcess } from "./sketchUpProcess";

export class SuDebugAdapterDescriptorFactory implements vscode.DebugAdapterDescriptorFactory {
	private sketchUpProcess: SketchUpProcess | undefined = undefined;

	createDebugAdapterDescriptor(session: vscode.DebugSession, executable: vscode.DebugAdapterExecutable | undefined): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {
		// use the executable specified in the package.json if it exists
		// or determine it based on some other information(e.g.the session)
		if (session.configuration.request === "launch") {
			this.launchSketchUp(session.configuration as SuDebugConfiguration);
			session.configuration.request = "attach";
		}

		if (!executable) {
			let dap = this.getDAP(session.configuration);
			executable = new vscode.DebugAdapterExecutable(dap.command, dap.args);
		}
		// make VS Code launch the DA executable
		return executable;
	}

	dispose() {
		if (this.sketchUpProcess) {
			this.sketchUpProcess.stop();
			this.sketchUpProcess = undefined;
		}
	}

	private launchSketchUp(configuration: SuDebugConfiguration) {
		if (this.sketchUpProcess === undefined) {
			if (!configuration.sketchUpProgram) {
				vscode.window.showErrorMessage("[SketchUpProgram] not informed in debug configuration");
			} else {
				this.sketchUpProcess = new SketchUpProcess(configuration.sketchUpProgram, configuration.sketchUpArguments);
				this.sketchUpProcess.launch(configuration.remotePort);
			}
		} else {
			vscode.window.showInformationMessage("sketchUp already running");
		}
	}

	private getDAP(configuration: vscode.DebugConfiguration): { command: string, args: string[] } {
		let pathDAP: string = "";
		let args: string[] = [];

		let ext = vscode.extensions.getExtension("brodao-lab.aclab-su-rdebug");
		if (!ext) {
			throw new Error("Extension [brodao-lab.aclab-su-rdebug] not initialize.");
		}

		if (process.platform === "win32") {
			pathDAP = path.join(
				ext.extensionPath,
				"/bin/windows/dap-rdebug.exe"
			);
		} else if (process.platform === "linux") {
			pathDAP = path.join(
				ext.extensionPath,
				"/bin/linux/dap-rdebug"
			);
			// if (statSync(pathDAP).mode !== 33261) {
			// 	chmodSync(pathDAP, "755");
			// }
		} else if (process.platform === "darwin") {
			pathDAP = path.join(
				ext.extensionPath,
				".bin/mac/dap-rdebug"
			);
			// if (statSync(pathDAP).mode !== 33261) {
			// 	chmodSync(pathDAP, "755");
			// }
		}

		const suConfiguration: SuDebugConfiguration = configuration as SuDebugConfiguration;

		args.push(`--remote-port=${suConfiguration.remotePort}`);
		args.push(`--remote-workspace-root=${suConfiguration.remoteWorkspaceRoot}`);

		if (suConfiguration.level) {
			args.push(`--level=${path.join(suConfiguration.remoteWorkspaceRoot, suConfiguration.logToFile)}`);
		}

		if (suConfiguration.logToFile) {
			args.push(`--log-to-file=${path.join(suConfiguration.remoteWorkspaceRoot, suConfiguration.logToFile)}`);
		}

		return { command: pathDAP, args: args };
	}
}