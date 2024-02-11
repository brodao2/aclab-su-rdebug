#pragma once

#include "msg-abstract-request.h"

class MessageDisconnectRequest : MessageAbstractRequest {
public:
	static dap::DisconnectResponse Run(const dap::DisconnectRequest& message);
};