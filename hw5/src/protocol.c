#include <unistd.h>
#include <stdlib.h>
#include "debug.h"
#include "protocol.h"
#include "csapp.h"

/*
 * Send a packet, which consists of a fixed-size header followed by an
 * optional associated data payload.
 *
 * @param fd  The file descriptor on which packet is to be sent.
 * @param pkt  The fixed-size packet header, with multi-byte fields
 *   in network byte order
 * @param data  The data payload, or NULL, if there is none.
 * @return  0 in case of successful transmission, -1 otherwise.
 *   In the latter case, errno is set to indicate the error.
 *
 * All multi-byte fields in the packet are assumed to be in network byte order.
 */


int proto_send_packet(int fd, BRS_PACKET_HEADER *hdr, void *payload){
    uint16_t payload_size = ntohs(hdr -> size);
    debug("payload: %d", payload_size);
    /*type - a single byte no need to convert*/
    // hdr -> size = htons(hdr -> size);
    // hdr -> timestamp_sec = htonl(hdr -> timestamp_sec);
    // hdr -> timestamp_nsec = htonl(hdr -> timestamp_nsec);

    if(write(fd, hdr, sizeof(BRS_PACKET_HEADER)) < sizeof(BRS_PACKET_HEADER)) {
        debug("proto_recv_packet - fail writing header to wire");
        return -1;
    }

    if(payload != NULL && payload_size != 0) {
        int n;
        if((n = rio_writen(fd, payload, payload_size) )< payload_size) {
            debug("writen: %d", n);
            debug("proto_recv_packet - fail writing payload to wire");
            return -1;
        }
    }
    return 0;
}


 // * Receive a packet, blocking until one is available.
 // *
 // * @param fd  The file descriptor from which the packet is to be received.
 // * @param pkt  Pointer to caller-supplied storage for the fixed-size
 // *   portion of the packet.
 // * @param datap  Pointer to a variable into which to store a pointer to any
 // *   payload received.
 // * @return  0 in case of successful reception, -1 otherwise.  In the
 // *   latter case, errno is set to indicate the error.
 // *
 // * The returned packet has all multi-byte fields in network byte order.
 // * If the returned payload pointer is non-NULL, then the caller has the
 // * responsibility of freeing that storage.

int proto_recv_packet(int fd, BRS_PACKET_HEADER *hdr, void **payloadp){
    int readbit;
    if((readbit = read(fd, hdr, sizeof(BRS_PACKET_HEADER))) < sizeof(BRS_PACKET_HEADER)){
        debug("read size: %d", readbit);
        if(!readbit) debug("EOF on fd: %d", fd);
        else debug("proto_recv_packet - fail reading header");
        return -1;
    }
    /*type - a single byte no need to convert*/
    hdr -> size = ntohs(hdr -> size);
    hdr -> timestamp_sec = ntohl(hdr -> timestamp_sec);
    hdr -> timestamp_nsec = ntohl(hdr -> timestamp_nsec);

    if(payloadp != NULL && hdr -> size != 0) {
        if((*payloadp = (char *)malloc(hdr -> size)) == NULL) {
            debug("proto_recv_packet - fail to malloc for payload");
            return -1;
        }
        /*reach here iff malloc succeed, must free if read fails*/
        if(rio_readn(fd, *payloadp, hdr -> size) < hdr -> size) {
            debug("proto_recv_packet - fail to read payload");
            free(*payloadp);
            return -1;
        }
    }
    return 0;
}