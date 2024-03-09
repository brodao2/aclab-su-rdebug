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

export class SuDebugConfigurationProvider implements vscode.DebugConfigurationProvider {
  static TYPE: string = "aclab-su-rdebug";

  constructor() {

  }

  /**
   * Massage a debug configuration just before a debug session is being launched,
   * e.g. add all missing attributes to the debug configuration.
   */
  async resolveDebugConfiguration(
    folder: vscode.WorkspaceFolder | undefined,
    config: vscode.DebugConfiguration,
    token?: vscode.CancellationToken
  ): Promise<vscode.DebugConfiguration> {

    if (folder) {
      config.workspaceFolders = folder;
    } else {
      let workspaceFolders = vscode.workspace.workspaceFolders;
      if (workspaceFolders) {
        let wsPaths: string[] = new Array(workspaceFolders.length);

        workspaceFolders.forEach((workspaceFolder: vscode.WorkspaceFolder, index: number) => {
          const workspaceFolderPath = workspaceFolder.uri.fsPath;
          wsPaths[index] = workspaceFolderPath;

        });

        config.workspaceFolders = wsPaths;
      }
    }

    if (!config.cwd || config.cwd === "") {
      config.cwd = config.remoteWorkspaceRoot;
    }

    return Promise.resolve(config);
  }

  // protected initialize(config: vscode.DebugConfiguration) {
  //   //   config.type = SuDebugConfigurationProvider.TYPE;
  //   //   config.name = "._NAME"; 
  // }

  // protected finalize(config: vscode.DebugConfiguration) {

  //   return config;
  // }

  resolveDebugConfigurationWithSubstitutedVariables(
    folder: vscode.WorkspaceFolder | undefined,
    debugConfiguration: vscode.DebugConfiguration,
    token?: vscode.CancellationToken
  ): vscode.ProviderResult<vscode.DebugConfiguration> {
    if (token?.isCancellationRequested) {
      return undefined;
    }

    return debugConfiguration;
  }

  dispose() {
    // if (this._server) {
    //   this._server.close();
    // }
  }
}
