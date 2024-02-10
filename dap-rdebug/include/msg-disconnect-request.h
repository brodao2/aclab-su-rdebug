#include <dap/protocol.h>

class MessageDisconnectRequest {
public:
	static dap::DisconnectResponse Run(const dap::DisconnectRequest& message);
};