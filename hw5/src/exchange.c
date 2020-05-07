#include "exchange.h"
#include "csapp.h"
#include "debug.h"

static void *matchmaker();

typedef struct order_list{
    orderid_t orderid;
    TRADER *trader;
    BRS_PACKET_TYPE type;
    BRS_ORDER_INFO info;
    struct{
        struct order_list *NEXT;
        struct order_list *PREV;
    }links;
} ORDER_LIST;


typedef struct exchange {
    orderid_t ordercnt;
    funds_t bid;
    funds_t ask;
    funds_t last;
    ORDER_LIST *orders;//for cancel order purpose
    ORDER_LIST *ord_hd;/*never change after init*/
    ORDER_LIST *last_order;
}EXCHANGE;


pthread_cond_t ConVar;
pthread_mutex_t mutex;


/*
 * Initialize a new exchange.
 *
 * @return  the newly initialized exchange, or NULL if initialization failed.
 */
EXCHANGE *exchange_init(){
    pthread_t tid;
    EXCHANGE *exchange = (EXCHANGE *)Malloc(sizeof(EXCHANGE));
    exchange -> ordercnt = 0;
    exchange -> bid = 0;
    exchange -> ask = 0;
    exchange -> last = 0;
    ORDER_LIST *init_ord = (ORDER_LIST *)Malloc(sizeof(ORDER_LIST));
    init_ord -> orderid = -1;
    init_ord -> links.PREV = init_ord -> links.NEXT = NULL;
    exchange -> orders = init_ord;
    exchange -> ord_hd = init_ord;
    if(pthread_create(&tid, NULL, matchmaker, exchange)) return NULL;
    pthread_cond_init(&ConVar, NULL);
    pthread_mutex_init(&mutex, NULL);
    return exchange;
}

/*
 * Finalize an exchange, freeing all associated resources.
 *
 * @param xchg  The exchange to be finalized, which must not
 * be referenced again.
 */
void exchange_fini(EXCHANGE *xchg){
    ORDER_LIST *cur = xchg -> ord_hd;
    while(cur){
        ORDER_LIST *next = cur -> links.NEXT;
        Free(cur);
        cur = next;
    }
    Free(xchg);
    pthread_cond_destroy(&ConVar);
    pthread_mutex_destroy(&mutex);
}

/*
 * Get the current status of the exchange.
 */
void exchange_get_status(EXCHANGE *xchg, BRS_STATUS_INFO *infop){
    infop -> bid = htonl(xchg -> bid);
    infop -> ask = htonl(xchg -> ask);
    infop -> last = htonl(xchg -> last);
}


 // * Post a buy order on the exchange on behalf of a trader.
 // * The trader is stored with the order, and its reference count is
 // * increased by one to account for the stored pointer.
 // * Funds equal to the maximum possible cost of the order are
 // * encumbered by removing them from the trader's account.
 // * A POSTED packet containing details of the order is broadcast
 // * to all logged-in traders.
 // *
 // * @param xchg  The exchange to which the order is to be posted.
 // * @param trader  The trader on whose behalf the order is to be posted.
 // * @param quantity  The quantity to be bought.
 // * @param price  The maximum price to be paid per unit.
 // * @return  The order ID assigned to the new order, if successfully posted,
 // * otherwise 0.

orderid_t exchange_post_buy(EXCHANGE *xchg, TRADER *trader, quantity_t quantity,
                funds_t price){
    pthread_mutex_lock(&mutex);
    xchg -> ordercnt ++;
    /* do something that might make condition true */
    if(trader_decrease_balance(trader, price*quantity)){
        debug("**********balance low**********");
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    if(price > xchg -> bid) xchg -> bid = price;
    ORDER_LIST *new_order = (ORDER_LIST *)Malloc(sizeof(ORDER_LIST));
    new_order -> links.PREV = new_order -> links.NEXT = NULL;
    new_order -> orderid = xchg -> ordercnt;
    new_order -> trader = trader;
    new_order -> type = BRS_BUY_PKT;
    new_order -> info.quantity = quantity;
    new_order -> info.price = price;

    ORDER_LIST *cur = xchg -> ord_hd;
    while(cur -> links.NEXT){
        cur = cur -> links.NEXT;
    }
    cur -> links.NEXT = new_order;
    new_order -> links.PREV = cur;
    xchg -> last_order = new_order;
    trader = trader_ref(trader, "because an order has been placed");

    BRS_PACKET_HEADER *hdr = (BRS_PACKET_HEADER *)Malloc(sizeof(BRS_PACKET_HEADER));
    BRS_NOTIFY_INFO *note = (BRS_NOTIFY_INFO *)Malloc(sizeof(BRS_NOTIFY_INFO));
    hdr -> type = BRS_POSTED_PKT;
    struct timespec tsp;
    clock_gettime(CLOCK_MONOTONIC, &tsp);
    hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
    hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
    hdr -> size = htons(sizeof(*note));
    note -> seller = htonl(0);
    note -> buyer = htonl(new_order -> orderid);
    note -> quantity = htonl(new_order -> info.quantity);
    note -> price = htonl(new_order -> info.price);
    trader_broadcast_packet(hdr, note);
    Free(hdr);
    Free(note);

    pthread_cond_signal(&ConVar);
    pthread_mutex_unlock(&mutex);
    return new_order -> orderid;
}


 // * Post a sell order on the exchange on behalf of a trader.
 // * The trader is stored with the order, and its reference count is
 // * increased by one to account for the stored pointer.
 // * Inventory equal to the amount of the order is
 // * encumbered by removing it from the trader's account.
 // * A POSTED packet containing details of the order is broadcast
 // * to all logged-in traders.
 // *
 // * @param xchg  The exchange to which the order is to be posted.
 // * @param trader  The trader on whose behalf the order is to be posted.
 // * @param quantity  The quantity to be sold.
 // * @param price  The minimum sale price per unit.
 // * @return  The order ID assigned to the new order, if successfully posted,
 // * otherwise 0.

orderid_t exchange_post_sell(EXCHANGE *xchg, TRADER *trader, quantity_t quantity,
                 funds_t price){
    pthread_mutex_lock(&mutex);
    xchg -> ordercnt ++;
    /* do something that might make condition true */
    if(trader_decrease_inventory(trader, quantity)) {
        debug("*******inventory low**********");
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    if(xchg -> ask == 0){
        debug("init ask");
        xchg -> ask = price;
    }
    if(price < xchg -> ask) xchg -> ask = price;
    ORDER_LIST *new_order = (ORDER_LIST *)Malloc(sizeof(ORDER_LIST));
    new_order -> links.PREV = new_order -> links.NEXT = NULL;
    new_order -> orderid = xchg -> ordercnt;
    new_order -> trader = trader;
    new_order -> type = BRS_SELL_PKT;
    new_order -> info.quantity = quantity;
    new_order -> info.price = price;

    ORDER_LIST *cur = xchg -> ord_hd;
    while(cur -> links.NEXT){
        cur = cur -> links.NEXT;
    }
    cur -> links.NEXT = new_order;
    new_order -> links.PREV = cur;
    xchg -> last_order = new_order;
    trader = trader_ref(trader, "because an order has been placed");

    BRS_PACKET_HEADER *hdr = (BRS_PACKET_HEADER *)Malloc(sizeof(BRS_PACKET_HEADER));
    BRS_NOTIFY_INFO *note = (BRS_NOTIFY_INFO *)Malloc(sizeof(BRS_NOTIFY_INFO));
    hdr -> type = BRS_POSTED_PKT;
    struct timespec tsp;
    clock_gettime(CLOCK_MONOTONIC, &tsp);
    hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
    hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
    hdr -> size = htons(sizeof(*note));
    note -> seller = htonl(new_order -> orderid);
    note -> buyer = htonl(0);
    note -> quantity = htonl(new_order -> info.quantity);
    note -> price = htonl(new_order -> info.price);
    trader_broadcast_packet(hdr, note);
    Free(hdr);
    Free(note);

    pthread_cond_signal(&ConVar);
    pthread_mutex_unlock(&mutex);
    return new_order -> orderid;
}

/*
 * Attempt to cancel a pending order.
 * If successful, the quantity of the canceled order is returned in a variable,
 * and a CANCELED packet containing details of the canceled order is
 * broadcast to all logged-in traders.
 *
 * @param xchg  The exchange from which the order is to be cancelled.
 * @param trader  The trader cancelling the order is to be posted,
 * which must be the same as the trader who originally posted the order.
 * @param id  The order ID of the order to be cancelled.
 * @param quantity  Pointer to a variable in which to return the quantity
 * of the order that was canceled.  Note that this need not be the same as
 * the original order amount, as the order could have been partially
 * fulfilled by trades.
 * @return  0 if the order was successfully cancelled, -1 otherwise.
 * Note that cancellation might fail if a trade fulfills and removes the
 * order before this function attempts to cancel it.
 */
int exchange_cancel(EXCHANGE *xchg, TRADER *trader, orderid_t order,
            quantity_t *quantity){
    pthread_mutex_lock(&mutex);
    /* do something that might make condition true */

    ORDER_LIST *cur = xchg -> ord_hd;
    while(cur -> links.NEXT){
        cur = cur -> links.NEXT;
        if(cur -> orderid == order) break;
    }

    quantity_t rmd = cur -> info.quantity;
    if(!rmd)return -1;

    BRS_NOTIFY_INFO *note = (BRS_NOTIFY_INFO *)Malloc(sizeof(BRS_NOTIFY_INFO));
    note -> seller = htonl(0);
    note -> buyer = htonl(0);
    if (cur -> type == BRS_SELL_PKT) note -> seller = htonl(cur -> orderid);
    if (cur -> type == BRS_BUY_PKT) note -> buyer = htonl(cur -> orderid);
    note -> quantity = htonl(rmd);
    note -> price = htonl(0);


    BRS_PACKET_HEADER *hdr = (BRS_PACKET_HEADER *)Malloc(sizeof(BRS_PACKET_HEADER));
    hdr -> type = BRS_CANCELED_PKT;
    struct timespec tsp;
    clock_gettime(CLOCK_MONOTONIC, &tsp);
    hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
    hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
    hdr -> size = htons(sizeof(*note));
    trader_send_packet(trader, hdr, note);
    Free(hdr);
    Free(note);
    ORDER_LIST *prev = cur -> links.PREV;
    ORDER_LIST *next = cur -> links.NEXT;
    if(prev)prev -> links.NEXT = next;
    if(next)next -> links.PREV = prev;
    Free(cur);
    trader_unref(trader, "in order being freed");
    pthread_mutex_unlock(&mutex);
    return 0;
}


static void *matchmaker(void *xchg){
    Pthread_detach(pthread_self());
    debug("******this is match maker******");
    while(1){
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&ConVar, &mutex);
        debug("******match maker wakes up******");
        /**things to do**/
        BRS_PACKET_HEADER *hdr = (BRS_PACKET_HEADER *)Malloc(sizeof(BRS_PACKET_HEADER));
        BRS_NOTIFY_INFO *note = (BRS_NOTIFY_INFO *)Malloc(sizeof(BRS_NOTIFY_INFO));
        EXCHANGE *exchange = (EXCHANGE *)xchg;
        ORDER_LIST *cur = exchange -> ord_hd;
        BRS_PACKET_TYPE type_to_search = -1;
        debug("*********type: %d********", exchange -> last_order -> type);
        if(exchange -> last_order -> type == BRS_BUY_PKT){
            type_to_search = BRS_SELL_PKT;
            debug("********************type to search is sell******************");
        }else if(exchange -> last_order -> type == BRS_SELL_PKT){
            type_to_search = BRS_BUY_PKT;
            debug("********************type to search is buy******************");
        }
        while(cur -> links.NEXT){
            cur = cur -> links.NEXT;
            if(cur -> type == type_to_search){
                TRADER *asktrader = exchange -> last_order -> trader;
                quantity_t askquant = exchange -> last_order -> info.quantity;
                funds_t askprice = exchange -> last_order -> info.price;
                orderid_t askorder = exchange -> last_order -> orderid;

                TRADER *curtrader = cur -> trader;
                quantity_t curquant = cur -> info.quantity;
                funds_t curprice = cur -> info.price;
                orderid_t curorder = cur -> orderid;
                debug("*************************1************************");
                if(cur -> type == BRS_BUY_PKT&& curprice >= askprice){
                    debug("**************************selling***********************");
                    if(curquant > askquant){
                        trader_increase_inventory(curtrader, askquant);
                        trader_increase_balance(asktrader, askquant*askprice);

                        note -> seller = htonl(askorder);
                        note -> buyer = htonl(curorder);
                        note -> quantity = htonl(askquant);
                        note -> price = htonl(askprice);

                        hdr -> type = BRS_SOLD_PKT;
                        struct timespec tsp;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(asktrader, hdr, note);
                        trader_unref(asktrader, "in order being freed");

                        hdr -> type = BRS_BOUGHT_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(curtrader, hdr, note);

                        hdr -> type = BRS_TRADED_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_broadcast_packet(hdr, note);

                        cur -> info.quantity -= askquant;
                        exchange -> last = askprice;

                        ORDER_LIST *prev = exchange -> last_order -> links.PREV;
                        ORDER_LIST *next = exchange -> last_order -> links.NEXT;
                        if(prev)prev -> links.NEXT = next;
                        if(next)next -> links.PREV = prev;
                        Free(exchange -> last_order);
                    }
                    else if(curquant == askquant){
                        trader_increase_inventory(curtrader, askquant);
                        trader_increase_balance(asktrader, askquant*askprice);

                        note -> seller = htonl(askorder);
                        note -> buyer = htonl(curorder);
                        note -> quantity = htonl(askquant);
                        note -> price = htonl(askprice);

                        hdr -> type = BRS_SOLD_PKT;
                        struct timespec tsp;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(asktrader, hdr, note);
                        trader_unref(asktrader, "in order being freed");

                        hdr -> type = BRS_BOUGHT_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(curtrader, hdr, note);
                        trader_unref(curtrader, "in order being freed");

                        hdr -> type = BRS_TRADED_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_broadcast_packet(hdr, note);

                        exchange -> last = askprice;

                        ORDER_LIST *prev = exchange -> last_order -> links.PREV;
                        ORDER_LIST *next = exchange -> last_order -> links.NEXT;
                        if(prev)prev -> links.NEXT = next;
                        if(next)next -> links.PREV = prev;
                        Free(exchange -> last_order);
                        ORDER_LIST *curprev = cur -> links.PREV;
                        ORDER_LIST *curnext = cur -> links.NEXT;
                        if(curprev)curprev -> links.NEXT = curnext;
                        if(curnext)curnext -> links.PREV = curprev;
                        Free(cur);

                    }else{/**curquant < askquant**/
                        trader_increase_inventory(curtrader, curquant);
                        trader_increase_balance(asktrader, curquant*askprice);

                        note -> seller = htonl(askorder);
                        note -> buyer = htonl(curorder);
                        note -> quantity = htonl(curquant);
                        note -> price = htonl(askprice);

                        hdr -> type = BRS_SOLD_PKT;
                        struct timespec tsp;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(asktrader, hdr, note);

                        hdr -> type = BRS_BOUGHT_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(curtrader, hdr, note);
                        trader_unref(curtrader, "in order being freed");

                        hdr -> type = BRS_TRADED_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_broadcast_packet(hdr, note);

                        exchange -> last_order -> info.quantity -= curquant;
                        exchange -> last = askprice;

                        ORDER_LIST *curprev = cur -> links.PREV;
                        ORDER_LIST *curnext = cur -> links.NEXT;
                        if(curprev)curprev -> links.NEXT = curnext;
                        if(curnext)curnext -> links.PREV = curprev;
                        Free(cur);
                    }
                }else if(cur -> type == BRS_SELL_PKT && curprice <= askprice){
                    if(curquant > askquant){
                        trader_increase_balance(curtrader, askquant*curprice);
                        trader_increase_inventory(asktrader, askquant);

                        note -> seller = htonl(curorder);
                        note -> buyer = htonl(askorder);
                        note -> quantity = htonl(askquant);
                        note -> price = htonl(curprice);

                        hdr -> type = BRS_BOUGHT_PKT;
                        struct timespec tsp;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(asktrader, hdr, note);
                        trader_unref(asktrader, "in order being freed");

                        hdr -> type = BRS_SOLD_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(curtrader, hdr, note);

                        hdr -> type = BRS_TRADED_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_broadcast_packet(hdr, note);

                        cur -> info.quantity -= askquant;
                        exchange -> last = curprice;

                        ORDER_LIST *prev = exchange -> last_order -> links.PREV;
                        ORDER_LIST *next = exchange -> last_order -> links.NEXT;
                        if(prev)prev -> links.NEXT = next;
                        if(next)next -> links.PREV = prev;
                        Free(exchange -> last_order);

                    }else if(curquant == askquant){
                        trader_increase_balance(curtrader, askquant*curprice);
                        trader_increase_inventory(asktrader, askquant);

                        note -> seller = htonl(curorder);
                        note -> buyer = htonl(askorder);
                        note -> quantity = htonl(askquant);
                        note -> price = htonl(curprice);

                        hdr -> type = BRS_BOUGHT_PKT;
                        struct timespec tsp;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(asktrader, hdr, note);
                        trader_unref(asktrader, "in order being freed");

                        hdr -> type = BRS_SOLD_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(curtrader, hdr, note);
                        trader_unref(curtrader, "in order being freed");

                        hdr -> type = BRS_TRADED_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_broadcast_packet(hdr, note);

                        exchange -> last = curprice;

                        ORDER_LIST *prev = exchange -> last_order -> links.PREV;
                        ORDER_LIST *next = exchange -> last_order -> links.NEXT;
                        if(prev)prev -> links.NEXT = next;
                        if(next)next -> links.PREV = prev;
                        Free(exchange -> last_order);
                        ORDER_LIST *curprev = cur -> links.PREV;
                        ORDER_LIST *curnext = cur -> links.NEXT;
                        if(curprev)curprev -> links.NEXT = curnext;
                        if(curnext)curnext -> links.PREV = curprev;
                        Free(cur);

                    }else{/*curquant < askquant*/

                        trader_increase_balance(curtrader, curquant*curprice);
                        trader_increase_inventory(asktrader, curquant);

                        note -> seller = htonl(curorder);
                        note -> buyer = htonl(askorder);
                        note -> quantity = htonl(curquant);
                        note -> price = htonl(curprice);

                        hdr -> type = BRS_BOUGHT_PKT;
                        struct timespec tsp;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(asktrader, hdr, note);

                        hdr -> type = BRS_SOLD_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_send_packet(curtrader, hdr, note);
                        trader_unref(curtrader, "in order being freed");

                        hdr -> type = BRS_TRADED_PKT;
                        clock_gettime(CLOCK_MONOTONIC, &tsp);
                        hdr -> timestamp_sec = htonl(tsp.tv_sec);/*htonl()*/
                        hdr -> timestamp_nsec = htonl(tsp.tv_nsec);
                        hdr -> size = htons(sizeof(*note));
                        trader_broadcast_packet(hdr, note);

                        exchange -> last_order -> info.quantity -= curquant;
                        exchange -> last = curprice;

                        ORDER_LIST *curprev = cur -> links.PREV;
                        ORDER_LIST *curnext = cur -> links.NEXT;
                        if(curprev)curprev -> links.NEXT = curnext;
                        if(curnext)curnext -> links.PREV = curprev;
                        Free(cur);

                    }
                }
            }
        }
        Free(hdr);
        Free(note);
        pthread_mutex_unlock(&mutex);
        debug("******match maker sleep******");
    }
    return NULL;
}