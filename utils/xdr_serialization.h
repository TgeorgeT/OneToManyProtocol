#ifndef XDR_SERIALIZATION_H
#define XDR_SERIALIZATION_H

#include <rpc/xdr.h>
#include "protocol_consts.h"
#include "protocol_structs.h"

bool_t xdr_packet_data(XDR *xdrs, packet_data *data);

#endif
