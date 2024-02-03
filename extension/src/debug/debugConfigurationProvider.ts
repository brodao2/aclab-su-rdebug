import { DebugSession, ProviderResult, ExtensionContext, DebugAdapterTrackerFactory, DebugAdapterTracker, debug, workspace } from 'vscode';
import { SuDebugConfiguration } from './debugConfigurationInterface';

export class RDebugTrackerDescriptorFactory implements DebugAdapterTrackerFactory {

	createDebugAdapterTracker(session: DebugSession): ProviderResult<DebugAdapterTracker> {
		const configuration: SuDebugConfiguration = session.configuration as SuDebugConfiguration;

		const printMessage = (label: string, message: any) => {
			const trace: string = configuration.trace;

			if (trace === "off") {
				return;
			}

			const now: Date = new Date();
			let text: string = `TRACE: ${label}\n\t`;

			if (trace === "verbose") {
				text = `${text}${JSON.stringify(message, undefined, "  ")}`;
			} else if (message.hasOwnProperty("command")) {
				text = `${text}type: ${message.type} command: ${message.command}`;
			} else if (message.hasOwnProperty("type") && message.type === "event") {
				text = `${text}type: ${message.type} event: ${message.event} ${message.body?.description || message.body?.message || ""}`;
			} else {
				text = `${text}${JSON.stringify(message, undefined, "  ")}`;
			}

			console.log(text);
			debug.activeDebugConsole?.appendLine(text);
		};

		const tracker: DebugAdapterTracker = {
			onDidSendMessage: (message: any) => {
				printMessage("<<", message);
			},
			onWillReceiveMessage: (message: any) => {
				printMessage(">>", message);
			},
			onWillStartSession: () => {
				printMessage("##", { type: "event", command: "onWillStartSession" });
			},
			onWillStopSession: () => {
				printMessage("##", { type: "event", command: "onWillStopSession" });
			},
			onError: (error: Error) => {
				printMessage(" ", error);
			},
			onExit: (code: number | undefined, signal: string | undefined) => {
				printMessage("##", { type: "event", command: `exit code=${code || ""} signal=${signal || ""}` });
			},
		};

		return tracker;
	}
}