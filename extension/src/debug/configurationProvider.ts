import {
  WorkspaceFolder,
  DebugConfigurationProvider,
  DebugConfiguration,
  CancellationToken,
  window,
} from "vscode";
import * as vscode from "vscode";
import * as Net from "net";

export class SuDebugConfigurationProvider implements DebugConfigurationProvider {
  static TYPE: string = "aclab-su-rdebug";

  //protected _server?: Net.Server;

  constructor() { }

  /**
   * Massage a debug configuration just before a debug session is being launched,
   * e.g. add all missing attributes to the debug configuration.
   */
  async resolveDebugConfiguration(
    folder: WorkspaceFolder | undefined,
    config: DebugConfiguration,
    token?: CancellationToken
  ): Promise<DebugConfiguration> {

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

    if (!config.cwb || config.cwb === "") {
      config.cwb = config.extensionDevelopmentPath;
      window.showInformationMessage(
        vscode.l10n.t("Parameter CWB not informed. Setting to {0}", config.cwb)
      );
    }

    return Promise.resolve(config);
  }

  protected initialize(config: DebugConfiguration) {
  //   config.type = SuDebugConfigurationProvider.TYPE;
  //   config.name = "._NAME"; 
  }

  protected finalize(config: DebugConfiguration) {
    
    return config;
  }

  resolveDebugConfigurationWithSubstitutedVariables(
    folder: WorkspaceFolder | undefined,
    debugConfiguration: DebugConfiguration,
    token?: CancellationToken
  ): vscode.ProviderResult<DebugConfiguration> {
    if (token?.isCancellationRequested) {
      return undefined;
    }

    return this.finalize(debugConfiguration);
  }

  dispose() {
    // if (this._server) {
    //   this._server.close();
    // }
  }
}
