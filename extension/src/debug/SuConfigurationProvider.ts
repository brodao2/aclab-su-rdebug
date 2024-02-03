import * as vscode from "vscode";
import {
  WorkspaceFolder,
  DebugConfigurationProvider,
  DebugConfiguration,
  CancellationToken,
  window,
} from "vscode";
import * as Net from "net";
import { SuDebugConfiguration } from "./debugConfigurationInterface";

export class SuConfigurationProvider implements DebugConfigurationProvider {

  protected _server?: Net.Server;

  constructor() {
    //
  }

  /**
   * Massage a debug configuration just before a debug session is being launched,
   * e.g. add all missing attributes to the debug configuration.
   */
  async resolveDebugConfiguration(
    folder: WorkspaceFolder | undefined,
    config: DebugConfiguration,
    token?: CancellationToken
  ): Promise<DebugConfiguration> {
    const configuration: SuDebugConfiguration = config as SuDebugConfiguration;

    // if (!config.type && !config.request && !config.name) {
    //   const editor = window.activeTextEditor;
    //   if (editor && editor.document.languageId === "totvs-developer-studio") {
    //     this.initialize(config);
    //     config.request = "launch";
    //     config.program = "${command:AskForProgramName}";
    //   }
    // }

    if (folder) {
      configuration.workspaceFolders = folder;
    } else {
      let workspaceFolders = vscode.workspace.workspaceFolders;
      if (workspaceFolders) {
        let wsPaths = new Array(workspaceFolders.length);
        let i = 0;
        for (const workspaceFolder of workspaceFolders) {
          const workspaceFolderPath = workspaceFolder.uri.fsPath;
          wsPaths[i] = workspaceFolderPath;
          i++;
        }
        configuration.workspaceFolders = wsPaths;
      }
    }

    if (!configuration.cwb || configuration.cwb === "") {
      configuration.cwb = configuration.extensionDevelopmentPath;
      window.showInformationMessage(
        vscode.l10n.t("Parameter CWB not informed. Setting to {0}", configuration.cwb)
      );
    }

    // let setDapArgsArr: string[] = [];
    // if (config.logFile) {
    //   const ws: string = vscode.workspace.rootPath || "";
    //   setDapArgsArr.push(
    //     "--log-file=" + config.logFile.replace("${workspaceFolder}", ws)
    //   );
    // }
    // if (config.waitForAttach) {
    //   setDapArgsArr.push("--wait-for-attach=" + config.waitForAttach);
    // }

    // if (config.forceUtf8) {
    //   vscode.window.showInformationMessage(vscode.l10n.t("--forceUtf8 deprecated. Setting will be ignored."))
    //   //setDapArgsArr.push("--forceUtf8");
    // }

    // if (config.language) {
    //   if (config.language === "1") config.language = "por";
    //   else if (config.language === "2") config.language = "spa";
    //   else if (config.language === "3") config.language = "eng";
    //   else if (config.language === "4") config.language = "rus";
    //   setDapArgsArr.push("--language=" + config.language);
    // }

    //setDapArgs(setDapArgsArr);

    return Promise.resolve(config);
  }

  protected initialize(config: DebugConfiguration) {
    // config.type = TotvsConfigurationProvider._TYPE;
    // config.name = TotvsConfigurationProvider._NAME;
    // config.smartclientBin = TotvsConfigurationProvider._SC_BIN;
  }

  protected finalize(config: DebugConfiguration) {
    return config;
  }

  resolveDebugConfigurationWithSubstitutedVariables(
    folder: WorkspaceFolder | undefined,
    debugConfiguration: DebugConfiguration,
    token?: CancellationToken
  ): vscode.ProviderResult<DebugConfiguration> {
    if (token!.isCancellationRequested) {
      return undefined;
    }


    // if (!debugConfiguration.program) {
    //   window.showInformationMessage(
    //     vscode.l10n.t("Cannot find a program to debug")
    //   );
    //   return undefined; // abort launch
    // }

    // debugConfiguration.program = programArgs.program;
    // debugConfiguration.programArguments = programArgs.args;

    return this.finalize(debugConfiguration);
  }

  dispose() {
    if (this._server) {
      this._server.close();
    }
  }
}
