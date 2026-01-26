/* Wrapper header to expose MCP2515 API from tests/builds which include Core/Inc in their include paths.
 * This avoids duplicating the header and keeps implementation headers in Src/Communication.
 */

#ifndef MCP2515_H
#define MCP2515_H

#include "../Src/Communication/mcp2515.h"

#endif /* MCP2515_H */
