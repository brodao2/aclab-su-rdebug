#include <dap/protocol.h>

class MessageInitializeRequest {
public:
	static dap::InitializeResponse Run(const dap::InitializeRequest& message);
};