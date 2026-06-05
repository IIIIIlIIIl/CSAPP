#include<getopt.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include "cachelab.h"
#define ll unsigned long long
#define GETTAG(addr) ((addr)>>(s+b))
#define GETIND(addr) (((addr)>>b)&maskS)
#define GETOFF(addr) ((addr)&maskB)

struct Line{
    int valid;
    int time;
    ll tag;
};

struct Set{
    struct Line *lines;
}*Cache;

int miss,hit,evict,timer;
ll maskS,maskB;
ll s,E,b,S,t,B,m=64;

int find(ll addr){
    ll ind=GETIND(addr),tag=GETTAG(addr);
    for(int i=0;i<E;i++){
        if(Cache[ind].lines[i].valid==0)continue;
        if(Cache[ind].lines[i].tag==tag){
            Cache[ind].lines[i].time=timer;
            return 1;
        }
    }
    return 0;
}

void add(ll ind,ll tag){
    for(int i=0;i<E;i++){
        if(Cache[ind].lines[i].valid==0){
            Cache[ind].lines[i].valid=1;
            Cache[ind].lines[i].tag=tag;
            Cache[ind].lines[i].time=timer;
            return;
        }
    }
    int lru=0;evict++;
    for(int i=0;i<E;i++){
        if(Cache[ind].lines[i].time<Cache[ind].lines[lru].time)lru=i;
    }
    Cache[ind].lines[lru].tag=tag;
    Cache[ind].lines[lru].time=timer;
}

void load(ll addr){
    timer++;
    if(find(addr)){
        hit++;
    }else{
        miss++;
        add(GETIND(addr),GETTAG(addr));
    }
}

void store(ll addr){
    timer++;
    if(find(addr)){
        hit++;
    }else{
        miss++;
        add(GETIND(addr),GETTAG(addr));
    }
}

int main(int argc,char* argv[])
{
    char *tracefile=NULL;
    char buf[50];
    ll opt;
    while((opt=getopt(argc,argv,"s:E:b:t:"))!=-1){
        switch(opt){
            case 's':
                s=atoi(optarg);
                break;
            case 'E':
                E=atoi(optarg);
                break;
            case 'b':
                b=atoi(optarg);
                break;
            case 't':
                tracefile=optarg;
                break;
        }
    }
    S=1ll<<s;B=1ll<<b;
    t=m-b-s;
    printf("%llu\n",t);
    maskS=S-1;
    maskB=B-1;
    Cache=calloc(S,sizeof(struct Set));
    for(int i=0;i<S;i++){
        Cache[i].lines=calloc(E,sizeof(struct Line));
    }
    char op;
    ll addr;
    int size;
    FILE *fp=fopen(tracefile,"r");
    while(fgets(buf,sizeof(buf),fp)!=NULL){
        if(sscanf(buf," %c %llx,%d",&op,&addr,&size)==3){
            switch(op){
                case 'L':
                    load(addr);
                    break;
                case 'S':
                    store(addr);
                    break;
                case 'M':
                    load(addr);
                    store(addr);
                    break;
            }
        }
    }
    printSummary(hit,miss,evict);
    return 0;
}
