#pragma once

#include "msg-abstract-request.h"

class MessageBreakpointLocationsRequest : MessageAbstractRequest {
public:
	static dap::BreakpointLocationsResponse Run(const dap::BreakpointLocationsRequest& message);
};