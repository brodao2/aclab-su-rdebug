import { DebugConfiguration } from "vscode";

export type SU_RDEBUG = "aclab-su-rdebug";

export interface SuAttachConfiguration extends DebugConfiguration {
	type: SU_RDEBUG;
	request: "attach";
	remotePort: number;
	extensionDevelopmentPath: string;
	stopOnEntry: boolean;
	trace: boolean;
}

export interface SuLaunchConfiguration extends DebugConfiguration {
	type: SU_RDEBUG;
	request: "attach";
	remotePort: number;
	extensionDevelopmentPath: string;
	stopOnEntry: boolean;
	trace: boolean;
	sketchUpProgram: string;
	sketchUpArguments: string[];
}

