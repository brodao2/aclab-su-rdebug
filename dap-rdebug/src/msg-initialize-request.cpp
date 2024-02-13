#include "msg-initialize-request.h"

dap::InitializeResponse MessageInitializeRequest::Run(const dap::InitializeRequest& message) {
	dap::InitializeResponse response;

	response.supportsBreakpointLocationsRequest = true;
	response.supportsConditionalBreakpoints = true;
	//response.supportsCancelRequest = true;
	response.supportsEvaluateForHovers = true;
	//response.supportsLogPoints = true;
	response.supportsTerminateRequest = true;

	return response;
}
