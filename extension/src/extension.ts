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
import { SuDebugConfigurationProvider } from './debug/configurationProvider';
import { SuDebugAdapterDescriptorFactory, stopSketchUp } from './debug/debugAdapterDescriptorFactory';
import { SuDebugAdapterTrackerFactory } from './debug/debugTrackerDescriptorFactory';

const SU_TYPE_DEBUG: string = "aclab-su-rdebug";
let outputChannel: vscode.OutputChannel;

export function pp(obj: any, obj1?: any) {
	let text: string = JSON.stringify(obj);

	if (obj1) {
		text += "\n" + JSON.stringify(obj1, undefined, 2) + "\n";
	}

	outputChannel.appendLine(text);
}


export function activate(context: vscode.ExtensionContext) {
	outputChannel = vscode.window.createOutputChannel("su-debug");

	context.subscriptions.push(
		vscode.debug.registerDebugConfigurationProvider(SU_TYPE_DEBUG, new SuDebugConfigurationProvider()),
		vscode.debug.registerDebugAdapterDescriptorFactory(SU_TYPE_DEBUG, new SuDebugAdapterDescriptorFactory()),
		vscode.debug.registerDebugAdapterTrackerFactory(SU_TYPE_DEBUG, new SuDebugAdapterTrackerFactory())
	);

	// context.subscriptions.push(
	// 	vscode.debug.onDidReceiveDebugSessionCustomEvent(
	// 		(debugEvent: vscode.DebugSessionCustomEvent) => {
	// 			console.log("onDidReceiveDebugSessionCustomEvent");
	// 		}
	// 	)
	// );

	// context.subscriptions.push(
	// 	vscode.debug.onDidStartDebugSession(async (session: vscode.DebugSession) => {
	// 		const config = session.configuration;
	// 		//if (config.request !== "launch" || config.useTerminal || config.noDebug) { return; };
	// 		if (config.noDebug) {
	// 			return;
	// 		};
	// 	})
	// );

	context.subscriptions.push(
		vscode.debug.onDidTerminateDebugSession((session: vscode.DebugSession) => {
			if (session.type === "ac-lab-rdebug") {
				stopSketchUp();
			}
		})
	);

	context.subscriptions.push(
		vscode.debug.onDidChangeActiveDebugSession(
			(debugEvent: vscode.DebugSession | undefined) => {
				console.log("onDidChangeActiveDebugSession");
			}
		)
	);

	// context.subscriptions.push(
	// 	vscode.debug.onDidChangeBreakpoints(
	// 		(event: vscode.BreakpointsChangeEvent) => {
	// 			//vscode.debug.activeDebugSession?.customRequest("break");

	// 			console.log("onDidChangeBreakpoints");

	// 		}
	// 	)
	// );
}

export function deactivate() {
	//
}
