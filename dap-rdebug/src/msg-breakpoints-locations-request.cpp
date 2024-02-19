#include "msg-breakpoints-locations-request.h"

dap::BreakpointLocationsResponse MessageBreakpointLocationsRequest::Run(const dap::BreakpointLocationsRequest& message) {
	dap::BreakpointLocationsResponse response;
	dap::BreakpointLocation bp;

	bp.column = message.column;
	bp.line = message.line;
	bp.endColumn = message.endColumn;
	bp.endLine = message.endLine;

	response.breakpoints.push_back(bp);

	return response;
}
