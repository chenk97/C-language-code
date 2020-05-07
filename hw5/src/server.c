#include "trader.h"
#include "exchange.h"
#include "server.h"
#include "csapp.h"
#include "debug.h"

/*
 * Thread function for the thread that handles a particular client.
 *
 * @param  Pointer to a variable that holds the file descriptor for
 * the client connection.  This pointer must be freed once the file
 * descriptor has been retrieved.
 * @return  NULL
 *
 * This function executes a "service loop" that receives packets from
 * the client and dispatches to appropriate functions to carry out
 * the client's requests.  It also maintains information about whether
 * the client has logged in or not.  Until the client has logged in,
 * only LOGIN packets will be honored.  Once a client has logged in,
 * LOGIN packets will no longer be honored, but other packets will be.
 * The service loop ends when the network connection shuts down and
 * EOF is seen.  This could occur either as a result of the client
 * explicitly closing the connection, a timeout in the network causing
 * the connection to be closed, or the main thread of the server shutting
 * down the connection as part of graceful termination.
 */
void *brs_client_service(void *arg){
    int login = 0;
    int connfd = *((int*)arg);
    Free(arg);
    Pthread_detach(pthread_self());
    creg_register(client_registry, connfd);
    debug("[%d]Starting client service", connfd);
    TRADER *trader = NULL;
    while(1){
        void **payloadp = Malloc(2*sizeof(void *));
        *payloadp = NULL;
        *(payloadp + 1) = NULL;
        BRS_PACKET_HEADER *hdr = (BRS_PACKET_HEADER *)malloc(sizeof(BRS_PACKET_HEADER));
        if(!proto_recv_packet(connfd, hdr, payloadp)/*0 means receive*/){
            if(hdr -> type == BRS_LOGIN_PKT){
                if(!login){
                    trader = trader_login(connfd, *payloadp);
                    while(trader){
                        trader_send_ack(trader, NULL);
                        login = 1;
                        break;
                    }
                }else{
                    //debug("%s",(char*)*payloadp);
                    trader_send_nack(trader);
                }
            }
            else if(hdr -> type == BRS_STATUS_PKT){
                if(login){
                    BRS_STATUS_INFO *info = Malloc(sizeof(BRS_STATUS_INFO));
                    exchange_get_status(exchange, info);
                    trader_send_ack(trader, info);
                    Free(info);
                }else{
                    debug("login required");
                }
            }else if(hdr -> type == BRS_DEPOSIT_PKT){
                if(login){
                    BRS_FUNDS_INFO *fund = *payloadp;
                    trader_increase_balance(trader, ntohl(fund -> amount));
                    BRS_STATUS_INFO *info = Malloc(sizeof(BRS_STATUS_INFO));
                    exchange_get_status(exchange, info);
                    trader_send_ack(trader, info);
                    Free(info);
                }else{
                    debug("login required");
                }
            }else if(hdr -> type == BRS_WITHDRAW_PKT){
                if(login){
                    BRS_FUNDS_INFO *fund = *payloadp;
                    if(!trader_decrease_balance(trader, ntohl(fund -> amount))){
                        BRS_STATUS_INFO *info = Malloc(sizeof(BRS_STATUS_INFO));
                        exchange_get_status(exchange, info);
                        trader_send_ack(trader, info);
                        Free(info);
                    }else{
                        trader_send_nack(trader);
                    }
                }else{
                    debug("login required");
                }
            }else if(hdr -> type == BRS_ESCROW_PKT){
                if(login){
                    BRS_ESCROW_INFO *quant = *payloadp;
                    trader_increase_inventory(trader, ntohl(quant -> quantity));
                    BRS_STATUS_INFO *info = Malloc(sizeof(BRS_STATUS_INFO));
                    exchange_get_status(exchange, info);
                    trader_send_ack(trader, info);
                    Free(info);
                }else{
                    debug("login required");
                }
            }else if(hdr -> type == BRS_RELEASE_PKT){
                if(login){
                    BRS_ESCROW_INFO *quant = *payloadp;
                    if(!trader_decrease_inventory(trader, ntohl(quant -> quantity))){
                        BRS_STATUS_INFO *info = Malloc(sizeof(BRS_STATUS_INFO));
                        exchange_get_status(exchange, info);
                        trader_send_ack(trader, info);
                        Free(info);
                    }else{
                        trader_send_nack(trader);
                    }
                }else{
                    debug("login required");
                }
            }else if(hdr -> type == BRS_BUY_PKT){
                if(login){
                    BRS_ORDER_INFO *orderinfo = *payloadp;
                    int oid;
                    if((oid = exchange_post_buy(exchange, trader, ntohl(orderinfo -> quantity),ntohl(orderinfo -> price)))){
                        BRS_STATUS_INFO *info = Malloc(sizeof(BRS_STATUS_INFO));
                        info -> orderid = ntohl(oid);
                        exchange_get_status(exchange, info);
                        trader_send_ack(trader, info);
                        Free(info);
                    }else{
                        trader_send_nack(trader);
                    }
                }else{
                    debug("login required");
                }
            }else if(hdr -> type == BRS_SELL_PKT){
                if(login){
                    BRS_ORDER_INFO *orderinfo = *payloadp;
                    int oid;
                    if((oid = exchange_post_sell(exchange, trader, ntohl(orderinfo -> quantity),ntohl(orderinfo -> price)))){
                        BRS_STATUS_INFO *info = Malloc(sizeof(BRS_STATUS_INFO));
                        info -> orderid = ntohl(oid);
                        exchange_get_status(exchange, info);
                        trader_send_ack(trader, info);
                        Free(info);
                    }else{
                        trader_send_nack(trader);
                    }
                }else{
                    debug("login required");
                }
            }else if(hdr -> type == BRS_CANCEL_PKT){
                if(login){
                    BRS_CANCEL_INFO *cancelid = *payloadp;
                    quantity_t *quant = Malloc(sizeof(quantity_t));
                    if(!exchange_cancel(exchange, trader, ntohl(cancelid -> order), quant)){
                        BRS_STATUS_INFO *info = Malloc(sizeof(BRS_STATUS_INFO));
                        info -> quantity = *quant;
                        exchange_get_status(exchange, info);
                        info -> orderid = cancelid -> order;
                        trader_send_ack(trader, info);
                        Free(info);
                    }else{
                        trader_send_nack(trader);
                    }
                    Free(quant);
                }else{
                    debug("login required");
                }
            }
        }else{
            debug("[%d]Ending client service", connfd);
            trader_logout(trader);
            creg_unregister(client_registry, connfd);
            Close(connfd);
            return NULL;
        }
        Free(hdr);
        Free(payloadp);
    }
}
