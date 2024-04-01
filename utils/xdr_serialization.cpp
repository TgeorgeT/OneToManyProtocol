#include <rpc/xdr.h>
#include "protocol_consts.h"
#include "protocol_structs.h"

bool_t xdr_packet_data(XDR *xdrs, packet_data *data)
{

    if (!xdr_int(xdrs, &data->flags))
    {
        std::cout << "Error serializing flags\n";
        return FALSE;
    }
    if (!xdr_int(xdrs, &data->ack))
    {
        std::cout << "Error serializing ack\n";
        return FALSE;
    }
    if (!xdr_int(xdrs, &data->seq))
    {
        std::cout << "Error serializing seq\n";
        return FALSE;
    }
    if (!xdr_int(xdrs, &data->checksum))
    {
        std::cout << "Error serializing checksum\n";
        return FALSE;
    }
    if (!xdr_int(xdrs, &data->payload_len))
    {
        std::cout << "Error serializing payload_len\n";
        return FALSE;
    }
    if (!xdr_bytes(xdrs, (char **)&data->payload, (u_int *)&data->payload_len, MAX_TRANSMITTED_LEN - 20))
        return FALSE;
    return TRUE;
}
