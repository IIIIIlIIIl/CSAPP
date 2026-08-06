/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    "",
    ""
};

#define MAX(x,y) ((x)>(y)?(x):(y))

/* single word (4) or double word (8) alignment */
#define WSIZE 4
#define DSIZE 8
#define ALIGNMENT 8
#define CHUNKSIZE (1<<12)

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (DSIZE-1)) & ~0x7)
#define PACK(size,alloc) ((size)|(alloc))

#define GET(p)     (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p)=(val))

#define GET_SIZE(p)  (GET(p)& ~0x7)
#define GET_ALLOC(p) (GET(p)& 0x1)

#define HDRP(bp) ((char*)(bp)-WSIZE)
#define FTRP(bp) ((char*)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

#define NEXT_BLKP(bp) ((char*)(bp)+GET_SIZE(((char*)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp)-GET_SIZE(((char*)(bp)-DSIZE)))

static char* heap_listp;
static void* fit_bp=NULL;
static void* last_bp=NULL;

static void mm_check(){
    void* bp;
    long total=0;
    for(bp=heap_listp;GET_SIZE(HDRP(bp))>0;bp=NEXT_BLKP(bp)){
        total+=GET_SIZE(HDRP(bp));
        if(GET_ALLOC(HDRP(bp))==0&&GET_ALLOC(HDRP(NEXT_BLKP(bp)))==0){
            printf("not coalesce completely at %p and next %p\n",bp,NEXT_BLKP(bp));
            exit(0);
        }
        if((size_t)(NEXT_BLKP(bp)-(size_t)bp)!=GET_SIZE(HDRP(bp))){
            printf("size not equal at %p,size=%d ,nextbp-bp=%d\n",bp,GET_SIZE(HDRP(bp)),(size_t)(NEXT_BLKP(bp)-(size_t)bp));
            exit(0);
        }else{
            // printf("correct bp=%p,size=%d\n",bp,GET_SIZE(HDRP(bp)));
        }
    }
    if((long)((long)bp-(long)heap_listp)!=total){
        printf("heap not complete! heap_listp=%p bp=%p total=%ld size=%ld\n",heap_listp,bp,total,(long)((long)bp-(long)heap_listp));
        exit(0);
    }
}

static void* coalesce(void *bp){
    char *nextptr=NEXT_BLKP(bp);
    size_t prev_alloc=GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));
    size_t flag=0;
    if(bp==fit_bp||PREV_BLKP(bp)==fit_bp||NEXT_BLKP(bp)==fit_bp)flag=1;

    if(prev_alloc&&next_alloc)return bp;
    else if(prev_alloc&&!next_alloc){
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        if(nextptr==last_bp)last_bp=bp;
    }
    else if(!prev_alloc&&next_alloc){
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        if(last_bp==bp)last_bp=PREV_BLKP(bp);
        bp=PREV_BLKP(bp);
    }
    else{
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
        if(nextptr==last_bp)last_bp=bp;
    }
    if(flag)fit_bp=bp;
    return bp;
}

static void* extend_heap(size_t words){
    char *bp;
    size_t size;

    size=(words%2)?(words+1)*WSIZE:words*WSIZE;
    if((bp=mem_sbrk(size))==(void*)-1)return NULL;
    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));
    return last_bp=coalesce(bp);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_listp=mem_sbrk(4*WSIZE))==(void*)-1)return -1;
    PUT(heap_listp,0);
    PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(3*WSIZE),PACK(0,1));
    heap_listp+=(2*WSIZE);
    fit_bp=heap_listp;
    // printf("init=%p %p\n",fit_bp,heap_listp);
    if(extend_heap(CHUNKSIZE/WSIZE)==NULL)return -1;
    // printf("size=%d\n",GET_SIZE(HDRP(last_bp)));
    return 0;
}

static void* find_fit(size_t asize){
    // mm_check();
    // printf("fuck=%d %d %d %p %p\n",asize,GET_SIZE(HDRP(fit_bp)),GET_SIZE(HDRP(NEXT_BLKP(fit_bp))),fit_bp,heap_listp);
    fit_bp=NEXT_BLKP(fit_bp);
    if(GET_SIZE(HDRP(fit_bp))==0)fit_bp=heap_listp;
    void* begin_bp=fit_bp;
    // size_t times=0;
    while(1){
        // times++;
        // if(times>=500)exit(0);
        // printf("now=%p\n",fit_bp);
        if(!GET_ALLOC(HDRP(fit_bp))&&(asize<=GET_SIZE(HDRP(fit_bp)))){
            return fit_bp;
        }
        fit_bp=NEXT_BLKP(fit_bp);
        if(GET_SIZE(HDRP(fit_bp))==0)fit_bp=heap_listp;
        if(fit_bp==begin_bp){
            break;
        }
    }
    return NULL;
}

static void place(void *bp,size_t asize){
    fit_bp=bp;
    size_t csize=GET_SIZE(HDRP(bp));
    if((csize-asize)>=(2*DSIZE)){
        PUT(HDRP(bp),PACK(asize,1));
        PUT(FTRP(bp),PACK(asize,1));
        if(bp==last_bp)last_bp=NEXT_BLKP(bp);
        bp=NEXT_BLKP(bp);
        PUT(HDRP(bp),PACK(csize-asize,0));
        PUT(FTRP(bp),PACK(csize-asize,0));
    }else{
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    // mm_check();
    size_t asize,extendsize;
    char *bp;

    if(size==0)return NULL;

    if(size<=DSIZE)asize=2*DSIZE;
    else asize=DSIZE*((size+DSIZE+(DSIZE-1))/DSIZE);
    // printf("malloc %d\n",asize);
    if((bp=find_fit(asize))!=NULL){
        // printf("findfit=%p,heap=%p\n",bp,heap_listp);
        place(bp,asize);
        return bp;
    }

    extendsize=MAX(asize,CHUNKSIZE);
    if(GET_ALLOC(HDRP(last_bp))==0)extendsize-=GET_SIZE(HDRP(last_bp));
    // printf("size1=%d\n",extendsize);
    if((bp=extend_heap(extendsize/WSIZE))==NULL)return NULL;
    place(bp,asize);fit_bp=bp;
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size=GET_SIZE(HDRP(ptr));
    // printf("free at=%p,size=%d\n",ptr,size);
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);
}

void *place_new(void* oldptr,size_t copySize,size_t size){
    char *newptr=mm_malloc(size);
    if(newptr==NULL)return NULL;
    memcpy(newptr,oldptr,copySize);
    mm_free(oldptr);
    // printf("val=%d %d %p %p\n",size,copySize,oldptr,newptr);
    return newptr;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if(ptr==NULL)return mm_malloc(size);
    if(size==0){
        mm_free(ptr);
        return NULL;
    }
    // mm_check();
    void *oldptr = ptr;
    size_t copySize;
    size_t oldsize=size;
    
    copySize = GET_SIZE(HDRP(ptr))-DSIZE;
    // printf("size==%d %d\n",size,copySize);
    if (size < copySize){
        // if(size==4112)printf("asdf\n");
        size=ALIGN(size+8);
        size_t remain=copySize+DSIZE-size;
        if(remain>=2*DSIZE){
            PUT(HDRP(ptr),PACK(size,1));
            PUT(FTRP(ptr),PACK(size,1));
            ptr=NEXT_BLKP(ptr);
            PUT(HDRP(ptr),PACK(copySize+DSIZE-size,0));
            PUT(FTRP(ptr),PACK(copySize+DSIZE-size,0));
            coalesce(ptr);
        }
        return oldptr;
    }else if(size>copySize){
        void *nextptr=NEXT_BLKP(ptr);
        size_t nextsize=GET_SIZE(HDRP(nextptr));
        size_t nextalloc=GET_ALLOC(HDRP(nextptr));
        void* prevptr=PREV_BLKP(ptr);
        size_t prevsize=GET_SIZE(HDRP(prevptr));
        size_t prevalloc=GET_ALLOC(HDRP(prevptr));
        if(prevalloc&&nextalloc==0){
            // printf("fuck=%p %p %d %d %d\n",nextptr,ptr,nextsize,copySize,ALIGN(size+8));
            size=ALIGN(size+8);
            size_t total=nextsize+copySize+DSIZE;
            if(total>=size){
                size_t remain=total-size; 
                if(remain>=2*DSIZE){
                    PUT(HDRP(ptr),PACK(size,1));
                    PUT(FTRP(ptr),PACK(size,1));
                    ptr=NEXT_BLKP(ptr);
                    if(nextptr==fit_bp)fit_bp=ptr;
                    if(nextptr==last_bp)last_bp=ptr;
                    PUT(HDRP(ptr),PACK(remain,0));
                    PUT(FTRP(ptr),PACK(remain,0));
                }else{
                    PUT(HDRP(ptr),PACK(total,1));
                    PUT(FTRP(ptr),PACK(total,1));
                    if(nextptr==fit_bp)fit_bp=ptr;
                    if(nextptr==last_bp)last_bp=ptr;
                }
                return oldptr;
            }else if(nextptr==last_bp){
                extend_heap((size-total)/WSIZE);
                PUT(HDRP(ptr),PACK(total,1));
                PUT(FTRP(ptr),PACK(total,1));
                if(nextptr==fit_bp)fit_bp=ptr;
                last_bp=ptr;
                return ptr;
            }else{
                return place_new(oldptr,copySize,oldsize);
            }
        }
        if(nextalloc&&prevalloc==0){
            size=ALIGN(size+8);
            size_t total=prevsize+copySize+DSIZE;
            if(total>=size){
                size_t remain=total-size;
                memmove(prevptr,ptr,copySize);
                if(remain>=2*DSIZE){
                    PUT(HDRP(prevptr),PACK(size,1));
                    PUT(FTRP(prevptr),PACK(size,1));
                    ptr=NEXT_BLKP(prevptr);
                    if(oldptr==fit_bp)fit_bp=ptr;
                    if(oldptr==last_bp)last_bp=ptr;
                    PUT(HDRP(ptr),PACK(remain,0));
                    PUT(FTRP(ptr),PACK(remain,0));
                }else{
                    PUT(HDRP(prevptr),PACK(total,1));
                    PUT(FTRP(prevptr),PACK(total,1));
                    if(oldptr==fit_bp)fit_bp=prevptr;
                    if(oldptr==last_bp)last_bp=prevptr;
                }
                return prevptr;
            }else if(oldptr==last_bp){
                extend_heap((size-total)/WSIZE);
                memmove(prevptr,ptr,copySize);
                PUT(HDRP(prevptr),PACK(total,1));
                PUT(FTRP(prevptr),PACK(total,1));
                if(oldptr==fit_bp)fit_bp=prevptr;
                last_bp=prevptr;
                return prevptr;
            }else{
                return place_new(oldptr,copySize,oldsize);
            }
        }
        if(prevalloc==0&&nextalloc==0){
            size=ALIGN(size+8);
            size_t total=prevsize+copySize+DSIZE+nextsize;
            if(total>=size){
                size_t remain=total-size;
                memmove(prevptr,ptr,copySize);
                if(remain>=2*DSIZE){
                    PUT(HDRP(prevptr),PACK(size,1));
                    PUT(FTRP(prevptr),PACK(size,1));
                    ptr=NEXT_BLKP(prevptr);
                    if(nextptr==fit_bp||oldptr==fit_bp)fit_bp=ptr;
                    if(nextptr==last_bp)last_bp=ptr;
                    PUT(HDRP(ptr),PACK(remain,0));
                    PUT(FTRP(ptr),PACK(remain,0));
                }else{
                    PUT(HDRP(prevptr),PACK(total,1));
                    PUT(FTRP(prevptr),PACK(total,1));
                    if(nextptr==fit_bp||oldptr==fit_bp)fit_bp=prevptr;
                    if(nextptr==last_bp)last_bp=prevptr;
                }
                return prevptr;
            }else if(nextptr==last_bp){
                extend_heap((size-total)/WSIZE);
                memmove(prevptr,ptr,copySize);
                PUT(HDRP(prevptr),PACK(total,1));
                PUT(FTRP(prevptr),PACK(total,1));
                if(nextptr==fit_bp||oldptr==fit_bp)fit_bp=prevptr;
                last_bp=prevptr;
                return prevptr;
            }else{
                return place_new(oldptr,copySize,oldsize);
            }
        }
        return place_new(oldptr,copySize,size);
    }
    return ptr;
}