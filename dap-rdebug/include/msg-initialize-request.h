#pragma once

#include "msg-abstract-request.h"

class MessageInitializeRequest {
public:
	static dap::InitializeResponse Run(const dap::InitializeRequest& message);
};