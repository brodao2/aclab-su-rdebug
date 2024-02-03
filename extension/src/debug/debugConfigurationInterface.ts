import { DebugConfiguration } from "vscode";

export type SU_RDEBUG = "aclab-su-rdebug";

export interface SuDebugConfiguration extends DebugConfiguration {
  remotePort: number;
  extensionDevelopmentPath: string;
  trace: "off" | "message" | "verbose";
  lotToFile: string;
  sketchUpProgram: string;
  sketchUpArguments: string[];
}

