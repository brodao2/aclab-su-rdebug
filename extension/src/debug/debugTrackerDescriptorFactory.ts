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
				},
				onExit(code: number) {
					pp(`<<< OnExit >>>\n`, `Exit code: ${code}`);
				},
				onDidSendMessage(message: any) {
					if (session.configuration.trace === "verbose") {
						if (message.type === "event") {
							pp(`<<< DA->VSCode event ${message.event}\n`, message);
						} else {
							pp(`<<< DA->VSCode command ${message.command}\n`, message);
						}						
					} else {
						if (message.type === "event") {
							pp(`<<< DA->VSCode event ${message.event}`);
						} else {
							pp(`<<< DA->VSCode command ${message.command}`);
						}						
					}
				},
				onWillReceiveMessage(message: any) {
					if (session.configuration.trace === "verbose") {
						pp(`>>> VSCode->DA ${message.command}\n`, message);
					} else {
						pp(`>>> VSCode->DA ${message.command}`);
					}
				}
			};
			// } else {
			// 	tracker.onDidSendMessage = (message: any): void => {
			// 		self.publishMessage(message);
			// 	};
			// }
		}

		return tracker;
	}

	private publishMessage(message: any) {
		//this._emitter.fire(message);
	}
}
