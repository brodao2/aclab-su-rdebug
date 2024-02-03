import * as vscode from 'vscode';
import { pp } from '../extension';

export class SuDebugAdapterTrackerFactory implements vscode.DebugAdapterTrackerFactory {
    constructor(
       //public readonly _emitter: vscode.EventEmitter<any>,
    ) { }

    createDebugAdapterTracker(session: vscode.DebugSession): vscode.ProviderResult<vscode.DebugAdapterTracker> {
        const self = this;
        const tracker: vscode.DebugAdapterTracker = {
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

        if (session.configuration.trace !== "off") {
            tracker.onDidSendMessage = (message: any): void => {
                self.publishMessage(message);
                pp("[DA->VSCode] " + JSON.stringify(message));
            };
            tracker.onWillReceiveMessage = (message: any): void => {
                pp("[VSCode->DA] " + JSON.stringify(message));
            };
        } else {
            tracker.onDidSendMessage = (message: any): void => {
                self.publishMessage(message);
            };
        }
        return tracker;
    }

    private publishMessage(message: any) {
        //this._emitter.fire(message);
    }
}
