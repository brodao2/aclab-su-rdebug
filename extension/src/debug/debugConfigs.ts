import { DebugConfiguration } from "vscode";

export interface SuDebugConfiguration extends DebugConfiguration {
  remotePort: number;
  extensionDevelopmentPath: string;
  trace: "off" | "message" | "verbose";
  logToFile: string;
  sketchUpProgram: string;
  sketchUpArguments: string[];
  level: "critical" | "error" | "warn" | "info" | "debug" | "trace" | "off";
  verbose: boolean;
}

