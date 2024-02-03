import path from 'path';
import { DebugAdapterDescriptor, DebugAdapterDescriptorFactory, DebugAdapterExecutable, DebugConfiguration, DebugSession, ProviderResult, extensions } from 'vscode';
import { SuDebugConfiguration } from './debugConfigurationInterface';

export class SuAdapterDescriptorFactory implements DebugAdapterDescriptorFactory {

	createDebugAdapterDescriptor(session: DebugSession, executable: DebugAdapterExecutable | undefined): ProviderResult<DebugAdapterDescriptor> {
		if (!executable) {
			let dap: { command: string, args: string[] } = this.getDAP(session.configuration);
			executable = new DebugAdapterExecutable(dap.command, dap.args);
		}

		return executable;
	}

	private getDAP(configuration: DebugConfiguration): { command: string, args: string[] } {
		let pathDAP: string = "";
		let args: string[] = [];

		let ext = extensions.getExtension("brodao-lab.aclab-su-rdebug");
		if (!ext) {
			throw new Error("Extension [brodao-lab.aclab-su-rdebug] not initialize.");
		}

		if (process.platform === "win32") {
			pathDAP = path.join(
				ext.extensionPath,
				"/node_modules/brodao-lab/bin/windows/dap-rdebug.exe"
			);
		} else if (process.platform === "linux") {
			pathDAP = path.join(
				ext.extensionPath,
				"/node_modules/brodao-lab/bin/linux/dap-rdebug"
			);
			// if (statSync(pathDAP).mode !== 33261) {
			// 	chmodSync(pathDAP, "755");
			// }
		} else if (process.platform === "darwin") {
			pathDAP = path.join(
				ext.extensionPath,
				"/node_modules/brodao-lab.bin/mac/dap-rdebug"
			);
			// if (statSync(pathDAP).mode !== 33261) {
			// 	chmodSync(pathDAP, "755");
			// }
		}

		const suConfiguration: SuDebugConfiguration = configuration as SuDebugConfiguration;

		args.push(`--port=${suConfiguration.remotePort}`);
		args.push(`--extensionDevelopmentPath=${suConfiguration.extensionDevelopmentPath}`);

		if (suConfiguration.logToFile) {
			args.push(`--logToFile=${path.join(suConfiguration.extensionDevelopmentPath, suConfiguration.logToFile)}`);
		}

		if (suConfiguration.type === "launch") {
			args.push(`--sketchUpProgram=${suConfiguration.sketchUpArguments}`);
			const encode = (str: string): string => Buffer.from(str, 'binary').toString('base64');

			let buffer: string[] = [];
			suConfiguration.sketchUpArguments.forEach((sketchUpArgument: string, index: number) => {
				//elimina necessidade de tratar aspas, caracteres acentuados e outros
				buffer.push(encode(sketchUpArgument));
			});

			args.push(`--sketchUpArgument=${buffer}`);
		}

		return { command: pathDAP, args: args };
	}
}