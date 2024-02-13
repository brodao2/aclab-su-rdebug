import * as vscode from 'vscode';
import { pp } from '../extension';

export class SuDebugAdapterTrackerFactory implements vscode.DebugAdapterTrackerFactory {
	constructor(
		//public readonly _emitter: vscode.EventEmitter<any>,
	) { }

	createDebugAdapterTracker(session: vscode.DebugSession): vscode.ProviderResult<vscode.DebugAdapterTracker> {
		const self = this;
		let tracker: vscode.DebugAdapterTracker = {};

		if (session.configuration.trace !== "off") {
			tracker = {
				onWillStartSession(): void {
					pp("[Start session]\n" + JSON.stringify(session));
				},
				onWillStopSession(): void {
					pp("[Stop session]\n" + JSON.stringify(session));
					// const outputTerminal = outputTerminals.get(session.id);
					// if (outputTerminal) {
					//     outputTerminal.show();
					//     outputTerminals.delete(session.id);
					// }
				},
				onError(e: Error) {
					pp("[Error on session]\n" + e.name + ": " + e.message + "\ne: " + JSON.stringify(e));
				}
			};

			tracker.onError = (error: Error): void => {
				self.publishMessage(error);

				pp(`*** ERROR ***\n`, error);
			};

			tracker.onExit = (code: number): void => {
				self.publishMessage(code);

				pp(`<<< OnExit >>>\n`, `Exit code: ${code}`);
			};

			tracker.onDidSendMessage = (message: any): void => {
				self.publishMessage(message);

				if (session.configuration.trace === "verbose") {
					pp(`<<< DA->VSCode ${message.command}\n`, message);
				} else {
					pp(`<<< DA->VSCode ${message.command}`);
				}
			};

			tracker.onWillReceiveMessage = (message: any): void => {
				self.publishMessage(message);

				if (session.configuration.trace === "verbose") {
					pp(`>>> VSCode->DA ${message.command}\n`, message);
				} else {
					pp(`>>> VSCode->DA ${message.command}`);
				}

			};
		}
		// } else {
		// 	tracker.onDidSendMessage = (message: any): void => {
		// 		self.publishMessage(message);
		// 	};
		// }

		return tracker;
	}

	private publishMessage(message: any) {
		//this._emitter.fire(message);
	}
}