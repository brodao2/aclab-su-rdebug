#pragma once

#include "msg-abstract-request.h"

class MessageAttachRequest {
public:
	static dap::AttachResponse Run(const dap::AttachRequest& message);
};