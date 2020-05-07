/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"

static void *find_fit(size_t bSize);
static size_t roundSize(size_t size);
static int find_head(size_t bSize);
static void insert(sf_block *bp);
static void delete(sf_block *bp);
static void place(sf_block *bp, size_t bSize);
static void *heap_extend();
static void init_freelist();
static void init_heap();
static void *coalesce(sf_block *bp);
static int ptr_validity(void *pp);

/*
 * This is your implementation of sf_malloc. It acquires uninitialized memory that
 * is aligned and padded properly for the underlying system.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If size is 0, then NULL is returned without setting sf_errno.
 * If size is nonzero, then if the allocation is successful a pointer to a valid region of
 * memory of the requested size is returned.  If the allocation is not successful, then
 * NULL is returned and sf_errno is set to ENOMEM.
 */
/*any size>0 can be rounded up to 32*/
void *sf_malloc(size_t size) {
    if(size != 0){/*check the validity of block size*/
        size_t bSize = roundSize(size + sizeof(sf_header) + sizeof(sf_footer));/*blocksize with header and footer*/
        debug("*********bSize: %ld********", bSize);
        sf_block *bp;/*block pointer to return*/
        if(sf_mem_start() == sf_mem_end()){/*if the heap is uninitialized*/
            init_freelist();
            init_heap();
        }
    redo:
        if((bp = find_fit(bSize))!= NULL){/*if a freeblock >= bSize is found in list*/
            place(bp, bSize);
            debug("******malloc******");
            //sf_show_heap();
            return bp -> body.payload;
        }else{/*no fit block find*/
            /*extend heap and insert*/
            if((bp = heap_extend()) != NULL){
                goto redo;
            }
        }
    }
    return NULL;
}

static void init_heap(){
    sf_mem_grow();
    /*set prologue and epilogue*/
    int pgsz = sf_mem_end() - sf_mem_start();
    sf_prologue *pro = sf_mem_start();
    pro -> header = (sizeof(sf_prologue)-sizeof(pro -> padding1)) | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED;
    pro -> footer = pro -> header ^ sf_magic();
    sf_epilogue *epi = sf_mem_end() - sizeof(sf_epilogue);
    epi -> header = sizeof(sf_epilogue) | THIS_BLOCK_ALLOCATED;
    sf_block * freeblk = sf_mem_start() + sizeof(sf_prologue) - sizeof(pro -> padding1);
    freeblk -> header = pgsz - sizeof(sf_prologue) - sizeof(sf_epilogue);
    if(((freeblk -> prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED)) freeblk -> header = freeblk -> header | PREV_BLOCK_ALLOCATED;
    sf_footer *ft = sf_mem_end() - sizeof(sf_epilogue) - sizeof(sf_footer);
    *ft = freeblk -> header ^ sf_magic();
    insert(freeblk);
}

static void *heap_extend(){
    void *bp;
    if((bp = sf_mem_grow()) != NULL){
        /*move epilogue to end*/
        debug("******extend******");
        sf_block *newblk = bp - sizeof(sf_epilogue) - sizeof(sf_footer);
        /*new epilogue*/
        sf_epilogue *newepi = sf_mem_end() - sizeof(sf_epilogue);
        newepi -> header = sizeof(sf_epilogue) | THIS_BLOCK_ALLOCATED;
        size_t newSize = sf_mem_end() - bp;
        newblk -> header = newSize;
        sf_footer *ft = (void *)newblk + newSize;
        /*new epilogue*/
        if(((newblk -> prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED) == 1) {
            newblk -> header = newblk -> header | PREV_BLOCK_ALLOCATED;
            *ft = newblk -> header ^ sf_magic();
            insert(newblk);
        }else{
            *ft = newblk -> header ^ sf_magic();
            newblk = coalesce(newblk);
            insert(newblk);
        }
        //sf_show_blocks();
        return newblk;
    }
    return NULL;
}

static void init_freelist(){
    for(int i = 0; i <= 8; i ++){
        sf_block *head = &sf_free_list_heads[i];
        head->body.links.next = head;
        head->body.links.prev = head;
    }
}

static size_t roundSize(size_t size){/*round the size to multiple of 16 which >= 32*/
    return size <= 32 ? 32 : (size % 16) > 0 ? (size / 16 + 1) * 16 : (size / 16) * 16;
}

static int find_head(size_t bSize){
    for(int i = 0; i < 8; i ++){
        if(bSize <= (32 << i)){
            return i;
        }
    }
    /*if lager than any of the size above*/
    return 8;
}

static void *find_fit(size_t bSize){/*pointer to appropriate free block*/
    sf_block *cur;/*traverse from the head i in list*/
    int headIdx = find_head(bSize);
    for(; headIdx <= 8; headIdx ++){/*iterate through heads*/
        sf_block *head = &sf_free_list_heads[headIdx];
        for(cur = head -> body.links.next; cur -> header > 0; cur = cur -> body.links.next){
            /*check size in header*/
            size_t curSize = cur-> header & BLOCK_SIZE_MASK;
            if(curSize >= bSize){
                delete(cur);
                return cur;/*remove the block from list after allocate*/
            }
        }
    }
    return NULL;/*return null if no block is found*/
}

static void place(sf_block *bp, size_t bSize){
    debug("******place*******");
    size_t curSize = bp -> header & BLOCK_SIZE_MASK;
    size_t rmdSize = curSize - bSize;
    if((rmdSize >= 32) && (rmdSize%16 == 0)){
        /*split the remainder*/
        bp -> header = bSize;
        bp -> header = bp -> header | THIS_BLOCK_ALLOCATED;/*mark as allocated*/
        if(((bp -> prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED)) bp -> header = bp -> header | PREV_BLOCK_ALLOCATED;
        sf_footer *ft = (void *)bp + bSize;
        *ft = bp -> header ^ sf_magic();/*footer of bp*/
        sf_block *rmd = (void *)bp + bSize;/*move pointer by bSize*/
        rmd -> header = rmdSize;
        if(((rmd -> prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED)) rmd -> header = rmd -> header | PREV_BLOCK_ALLOCATED;
        sf_footer *rmdft= (void *)rmd + rmdSize;
        *rmdft = rmd -> header ^ sf_magic();/*footer of rmd*/
        /*update next block for sf_realloc*/
        sf_block *rmdnxt = (void *)rmd + rmdSize;
        size_t rmdnxtSize = rmdnxt -> header & BLOCK_SIZE_MASK;
        /*remainder is always free, update block next to remainder, this is for realloc purpose*/
        if(rmdnxt -> header & THIS_BLOCK_ALLOCATED){
                rmdnxt -> header = rmdnxtSize | THIS_BLOCK_ALLOCATED;
            }else{
                rmdnxt -> header = rmdnxtSize;
            }
        if(rmdnxtSize > 8){/*update footer if next block is not epilogue*/
            sf_footer *rmdNxtft= (void *)rmdnxt + rmdnxtSize;
            *rmdNxtft = rmdnxt -> header ^ sf_magic();
        }
        coalesce(rmd);
        insert(rmd);
    }else{
        /*allocate block without split*/
        bp -> header = curSize;
        bp -> header = bp -> header|THIS_BLOCK_ALLOCATED;/*mark as allocated*/
        if((bp -> prev_footer ^ sf_magic()) & THIS_BLOCK_ALLOCATED) bp -> header = bp -> header | PREV_BLOCK_ALLOCATED;
        sf_footer *ft = (void *)bp + curSize;
        *ft = bp -> header ^ sf_magic();
        /*update next block, as this block is allocated*/
        sf_block *next = (void *)bp + curSize;
        size_t nextSize = next -> header & BLOCK_SIZE_MASK;
        if(next -> header & THIS_BLOCK_ALLOCATED){
                next -> header = nextSize | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED;;
            }else{
                next -> header = nextSize | PREV_BLOCK_ALLOCATED;;
            }
        if(nextSize > 8){/*update footer if next block is not epilogue*/
            sf_footer *nextft = (void *)next + nextSize;
            *nextft = next -> header ^ sf_magic();
        }
    }
}

/*
 * Marks a dynamically allocated region as no longer in use.
 * Adds the newly freed block to the free list.
 *
 * @param ptr Address of memory returned by the function sf_malloc.
 *
 * If ptr is invalid, the function calls abort() to exit the program.
 */

/*
 * INVALID POINTER:
 * The pointer is NULL.
 * The allocated bit in the header is 0.
 * The header of the block is before the end of the prologue, or the footer
 * of the block is after the beginning of the epilogue.
 * The block_size field is less than the minimum block size of 32 bytes.
 * The prev_alloc field is 0, indicating that the previous block is free,but the alloc field of the previous block header is not 0.
 * The bitwise XOR of the footer contents and the value returned by sf_magic() does not equal the header contents.
 */
void sf_free(void *pp) {
    /*pp is addr of payload*/
    /*check pointer validity for abortion*/
    if(!ptr_validity(pp)) abort();
    sf_block *bp = pp - sizeof(sf_footer) - sizeof(sf_header);
    /*check pointer validity for abortion*/
    bp = (sf_block *)coalesce(bp);
    insert(bp);
    debug("************free***********");
    //sf_show_heap();
    return;
}

static int ptr_validity(void *pp){
    if(pp == NULL) return 0;
    sf_block *bp = pp - sizeof(sf_footer) - sizeof(sf_header);
    size_t bpSize = bp -> header & BLOCK_SIZE_MASK;
    sf_footer *ft = (void *)bp + bpSize;
    sf_footer *proft = sf_mem_start() +sizeof(sf_prologue) - sizeof(sf_footer);
    sf_header *epihd = sf_mem_end() - sizeof(sf_epilogue);
    if((bp -> header & THIS_BLOCK_ALLOCATED) == 0 || (bp -> header & BLOCK_SIZE_MASK) < 32) return 0;
    if((void *)&(bp -> header) < ((void *)proft + sizeof(sf_footer))) return 0;
    if(((void *)ft + sizeof(sf_footer)) > (void *)epihd) return 0;
    size_t prev_alloc = ((bp -> prev_footer) ^ sf_magic()) & THIS_BLOCK_ALLOCATED;
    size_t this_prevalloc = bp -> header & PREV_BLOCK_ALLOCATED;
    if(this_prevalloc == 0 && prev_alloc != 0) return 0;
    if((*ft ^ sf_magic()) != bp -> header) return 0;
    return 1;
}

static void *coalesce(sf_block *bp){
    size_t bpSize = bp -> header & BLOCK_SIZE_MASK;
    size_t prevSize = ((bp -> prev_footer) ^ sf_magic()) & BLOCK_SIZE_MASK;
    size_t prev_alloc = ((bp -> prev_footer) ^ sf_magic()) & THIS_BLOCK_ALLOCATED;
    sf_block *preced = (void *)bp - prevSize;
    sf_block *succed = (void *)bp + bpSize;
    size_t nextSize = succed -> header & BLOCK_SIZE_MASK;
    size_t next_alloc = succed -> header & THIS_BLOCK_ALLOCATED;
    if(prev_alloc && next_alloc){
        bp -> header = bpSize|PREV_BLOCK_ALLOCATED;
        sf_footer *ft = (void *)bp + bpSize;
        *ft = (bp -> header) ^ sf_magic();
        /*update the status of next*/
        succed -> header = nextSize | THIS_BLOCK_ALLOCATED;
        if(nextSize > 8){
            sf_footer *nextft = (void *) succed + nextSize;
            *nextft = succed -> header ^ sf_magic();
        }
    }else if(prev_alloc && !next_alloc){
        delete(succed);/*next*/
        bpSize += nextSize;
        bp -> header = bpSize;
        sf_footer *ft = (void *)bp + bpSize;
        bp -> header = bp -> header | PREV_BLOCK_ALLOCATED;
        *ft = (bp -> header) ^ sf_magic();
    }
    else if(!prev_alloc && next_alloc){
        delete(preced);
        bpSize += prevSize;
        bp = (void *)bp - prevSize;/*move pointer*/
        if(preced -> header && PREV_BLOCK_ALLOCATED){
            bp -> header = bpSize | PREV_BLOCK_ALLOCATED;
        }else bp -> header = bpSize;
        sf_footer *ft = (void *)bp + bpSize;
        *ft = (bp -> header) ^ sf_magic();
        succed -> header = nextSize | THIS_BLOCK_ALLOCATED;
        if(nextSize > 8){
            sf_footer *nextft = (void *) succed + nextSize;
            *nextft = succed -> header ^ sf_magic();
        }
    }
    else{
        delete(preced);
        delete(succed);
        bpSize += prevSize + nextSize;
        bp = (void *)bp - prevSize;/*move pointer*/
        if(preced -> header && PREV_BLOCK_ALLOCATED){
            bp -> header = bpSize | PREV_BLOCK_ALLOCATED;
        }else bp -> header = bpSize;
        sf_footer *ft = (void *)bp + bpSize;
        *ft = (bp -> header) ^ sf_magic();
    }
    return bp;
}

static void insert(sf_block *bp){
    size_t bpSize = bp -> header & BLOCK_SIZE_MASK;
    sf_block *head = &sf_free_list_heads[find_head(bpSize)];
    bp -> body.links.prev = head;
    bp -> body.links.next = head -> body.links.next;
    head -> body.links.next -> body.links.prev = bp;
    head -> body.links.next = bp;
}

static void delete(sf_block *bp){
    sf_block *bpNext = bp -> body.links.next;
    sf_block *bpPrev = bp -> body.links.prev;
    bpPrev -> body.links.next = bpNext;
    bpNext -> body.links.prev = bpPrev;
}

/*
 * Resizes the memory pointed to by ptr to size bytes.
 *
 * @param ptr Address of the memory region to resize.
 * @param size The minimum size to resize the memory to.
 *
 * @return If successful, the pointer to a valid region of memory is
 * returned, else NULL is returned and sf_errno is set appropriately.
 *
 *   If sf_realloc is called with an invalid pointer sf_errno should be set to EINVAL.
 *   If there is no memory available sf_realloc should set sf_errno to ENOMEM.
 *
 * If sf_realloc is called with a valid pointer and a size of 0 it should free
 * the allocated block and return NULL without setting sf_errno.
 */
void *sf_realloc(void *pp, size_t rsize) {
    if(!ptr_validity(pp)){
        sf_errno = EINVAL;
        return NULL;
    }
    sf_block *bp = pp - sizeof(sf_footer) - sizeof(sf_header);
    size_t bpSize = bp -> header & BLOCK_SIZE_MASK;
    if(rsize == 0){
        sf_free(pp);
        return NULL;
    }
    size_t roundRsize = roundSize(rsize + sizeof(sf_header) + sizeof(sf_footer));
    if(bpSize < roundRsize){
        char *dest = sf_malloc(rsize);
        if(!dest) return NULL;
        memcpy(dest, bp -> body.payload, bpSize);
        sf_free(pp);
        return dest;
    }else{
        place(bp, roundRsize);
        return bp -> body.payload;
    }
    return NULL;
}
