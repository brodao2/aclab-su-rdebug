#pragma once

#include "msg-abstract-request.h"

class MessageLaunchRequest {
public:
	static dap::LaunchResponse Run(const dap::LaunchRequest& message);
};