#include <rpc/xdr.h>
#include "protocol_consts.h"
#include "protocol_structs.h"

bool_t xdr_packet_data(XDR *xdrs, packet_data *data)
{
    if (!xdr_int(xdrs, &data->flags) ||
        !xdr_int(xdrs, &data->ack) ||
        !xdr_int(xdrs, &data->seq) ||
        !xdr_int(xdrs, &data->checksum) ||
        !xdr_int(xdrs, &data->payload_len))
        return FALSE;
    if (!xdr_bytes(xdrs, (char **)&data->payload, (u_int *)&data->payload_len, MAX_TRANSMITTED_LEN - 20))
        return FALSE;
    return TRUE;
}
