#include "msg-initialize-request.h"

dap::InitializeResponse MessageInitializeRequest::Run(const dap::InitializeRequest& message) {
	dap::InitializeResponse response;

	// The debug adapter supports the `suspendDebuggee` attribute on the `disconnect` request.
	response.supportSuspendDebuggee = true;
	// The debug adapter supports the `terminateDebuggee` attribute on the `disconnect` request.
	response.supportTerminateDebuggee = true;
	// The debug adapter supports the `breakpointLocations` request.
	response.supportsBreakpointLocationsRequest = false;
	// The debug adapter supports conditional breakpoints.
	response.supportsConditionalBreakpoints = true;
	// The debug adapter supports the `configurationDone` request.
	response.supportsConfigurationDoneRequest = true;
	// The debug adapter supports a (side effect free) `evaluate` request for data hovers.
	response.supportsEvaluateForHovers = true;
	// The debug adapter supports setting a variable to a value.
	response.supportsSetVariable = true;
	// The debug adapter supports the `singleThread` property on the execution
	// requests (`continue`, `next`, `stepIn`, `stepOut`, `reverseContinue`,
	// `stepBack`).
	response.supportsSingleThreadExecutionRequests = true;
	// The debug adapter supports the `terminate` request.
	response.supportsTerminateRequest = true;




	// The set of additional module information exposed by the debug adapter.
	response.additionalModuleColumns = {};
	// The set of characters that should trigger completion in a REPL. If not specified, the UI should assume the `.` character.
	response.completionTriggerCharacters = {};
	// Available exception filter options for the `setExceptionBreakpoints` request.
	response.exceptionBreakpointFilters = {};
	// Checksum algorithms supported by the debug adapter.
	response.supportedChecksumAlgorithms = {};
	// The debug adapter supports the `cancel` request.
	response.supportsCancelRequest = false;
	// The debug adapter supports the `clipboard` context value in the `evaluate` request.
	response.supportsClipboardContext = false;
	// The debug adapter supports the `completions` request.
	response.supportsCompletionsRequest = false;


	// The debug adapter supports data breakpoints.
	response.supportsDataBreakpoints = false;
	// The debug adapter supports the delayed loading of parts of the stack, which requires that both the `startFrame` and `levels` arguments and the `totalFrames` result of the `stackTrace` request are supported.
	response.supportsDelayedStackTraceLoading = false;
	// The debug adapter supports the `disassemble` request.
	response.supportsDisassembleRequest = false;




	// The debug adapter supports `filterOptions` as an argument on the `setExceptionBreakpoints` request.
	response.supportsExceptionFilterOptions = false;
	// The debug adapter supports the `exceptionInfo` request.
	response.supportsExceptionInfoRequest = false;
	// The debug adapter supports `exceptionOptions` on the `setExceptionBreakpoints` request.
	response.supportsExceptionOptions = false;
	// The debug adapter supports function breakpoints.
	response.supportsFunctionBreakpoints = false;
	// The debug adapter supports the `gotoTargets` request.
	response.supportsGotoTargetsRequest = false;
	// The debug adapter supports breakpoints that break execution after a specified number of hits.
	response.supportsHitConditionalBreakpoints = false;
	// The debug adapter supports adding breakpoints based on instruction references.
	response.supportsInstructionBreakpoints = false;
	// The debug adapter supports the `loadedSources` request.
	response.supportsLoadedSourcesRequest = false;
	// The debug adapter supports log points by interpreting the `logMessage` attribute of the `SourceBreakpoint`.
	response.supportsLogPoints = false;
	// The debug adapter supports the `modules` request.
	response.supportsModulesRequest = false;
	// The debug adapter supports the `readMemory` request.
	response.supportsReadMemoryRequest = false;
	// The debug adapter supports restarting a frame.
	response.supportsRestartFrame = false;
	// The debug adapter supports the `restart` request. In this case a client
	// should not implement `restart` by terminating and relaunching the adapter
	// but by calling the `restart` request.
	response.supportsRestartRequest = false;
	// The debug adapter supports the `setExpression` request.
	response.supportsSetExpression = false;






	// The debug adapter supports stepping back via the `stepBack` and `reverseContinue` requests.
	response.supportsStepBack = false;
	// The debug adapter supports the `stepInTargets` request.
	response.supportsStepInTargetsRequest = false;
	// The debug adapter supports stepping granularities (argument `granularity`) for the stepping requests.
	response.supportsSteppingGranularity = false;
	// The debug adapter supports the `terminateThreads` request.
	response.supportsTerminateThreadsRequest = false;
	// The debug adapter supports a `format` attribute on the `stackTrace`, `variables`, and `evaluate` requests.
	response.supportsValueFormattingOptions = false;
	// The debug adapter supports the `writeMemory` request.
	response.supportsWriteMemoryRequest = false;
	
	return response;
}
