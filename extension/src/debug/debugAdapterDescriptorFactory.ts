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

import * as vscode from "vscode";
import * as path from "path";
import { SuDebugConfiguration, getSuConfiguration } from "./debugConfigs";
import { SketchUpProcess } from "./sketchUpProcess";

let sketchUpProcess: SketchUpProcess | undefined = undefined;

export class SuDebugAdapterDescriptorFactory implements vscode.DebugAdapterDescriptorFactory {

	createDebugAdapterDescriptor(session: vscode.DebugSession, executable: vscode.DebugAdapterExecutable | undefined): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {
		// use the executable specified in the package.json if it exists
		// or determine it based on some other information(e.g.the session)
		if (session.configuration.request === "launch") {
			launchSketchUp(session.configuration as SuDebugConfiguration);
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
		// if (this.sketchUpProcess) {
		// 	this.sketchUpProcess.stop();
		// 	this.sketchUpProcess = undefined;
		// }
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

		if (getSuConfiguration().logToFile) {
			if (suConfiguration.level) {
				args.push(`--level=${path.join(suConfiguration.remoteWorkspaceRoot, getSuConfiguration().logToFile)}`);
			}

			args.push(`--log-to-file=${path.join(suConfiguration.remoteWorkspaceRoot, getSuConfiguration().logToFile)}`);
		}

		return { command: pathDAP, args: args };
	}
}

function launchSketchUp(configuration: SuDebugConfiguration) {
	if (sketchUpProcess === undefined) {
		if (!configuration.sketchUpProgram) {
			vscode.window.showErrorMessage("[SketchUpProgram] not informed in debug configuration");
		} else {
			sketchUpProcess = new SketchUpProcess(configuration.sketchUpProgram, configuration.sketchUpArguments);
			sketchUpProcess.launch(configuration.remotePort);
		}
	} else {
		vscode.window.showInformationMessage("SketchUp already running");
	}
}

export function stopSketchUp() {
	if (sketchUpProcess !== undefined) {
		sketchUpProcess.stop();
	}
}
