// PlayerBots | Nexys.Tuga | Linux x64
// Compativel especificamente com net.so MTA 1.6.0-9.23312.0 (tested SHA-256: 39d4a4f1b0b5c51ca792dc45f134c695e9083ea6ca266d61a2c3e4e085fcb56d).
// Ajusta respostas ASE EYE e sincroniza o PingStatus usando o encoder original da net.so.
// Nao cria ligacoes/jogadores reais.

#include <dlfcn.h>
#include <link.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

struct lua_State;
typedef int (*lua_CFunction)(lua_State*);
class ILuaModuleManager10 {
public:
    virtual void ErrorPrintf(const char*, ...)=0;
    virtual void DebugPrintf(lua_State*,const char*,...)=0;
    virtual void Printf(const char*,...)=0;
    virtual bool RegisterFunction(lua_State*,const char*,lua_CFunction)=0;
};

static ILuaModuleManager10* g_manager=nullptr;

using SendTo = ssize_t (*)(int,const void*,size_t,int,const struct sockaddr*,socklen_t);
static SendTo g_original_sendto=nullptr;
static bool g_enabled=false,g_hooked=false;
static unsigned g_attempts=0;

static unsigned g_target=0,g_pending=0,g_digits=0;
static bool g_receiving=false;

static const unsigned MAX_NAMES=128;
static const unsigned MAX_NAME_BYTES=48;
static char g_names[MAX_NAMES][MAX_NAME_BYTES+1];
static unsigned g_name_len[MAX_NAMES];
static unsigned g_name_count=0;
static char g_pending_name[MAX_NAME_BYTES+1];
static unsigned g_pending_name_len=0;
static int g_half_nibble=-1;
static bool g_names_receiving=false,g_name_receiving=false;

#pragma pack(push,1)
struct PingInputV1 {
    uint8_t type;
    uint8_t pad[3];
    uint32_t unused;
    uint16_t marker;
    uint16_t players;
};
#pragma pack(pop)

// Layout da std::string C++11 do libstdc++ x86_64.
// O encoder da net.so recebe precisamente este tipo como segundo argumento.
struct GnuString64 {
    char* ptr;
    size_t size;
    union {
        char local[16];
        size_t capacity;
    } storage;
};

using PingEncoder = void (*)(const void*, GnuString64*);
static PingEncoder g_ping_encoder=nullptr;
static bool g_ping_encoder_checked=false,g_ping_encoder_ok=false;
static uintptr_t g_net_base=0;
static char g_net_version[64]={0};

struct GotPatch { void** slot; void* original; int prot; };
static GotPatch g_patches[16];
static unsigned g_patch_count=0;

static int eye_index(const char* b,int n){
    return b&&n>=4&&b[0]=='E'&&b[1]=='Y'&&b[2]=='E'&&b[3]>='1'&&b[3]<='3' ? b[3]-'1' : -1;
}

static void copy_info(char* dst,const char* src){
    if(!dst||!src)return;
    unsigned i=0; while(i<127&&src[i]){dst[i]=src[i];++i;} dst[i]=0;
}

static const char* base_name(const char* p){
    if(!p)return ""; const char* last=p;
    for(const char* q=p;*q;++q)if(*q=='/')last=q+1;
    return last;
}

struct FindModuleCtx { const char* name; uintptr_t base; const char* path; };
static int find_module_cb(struct dl_phdr_info* info,size_t,void* data){
    FindModuleCtx* c=(FindModuleCtx*)data;
    if(info&&info->dlpi_name&&strcmp(base_name(info->dlpi_name),c->name)==0){
        c->base=(uintptr_t)info->dlpi_addr; c->path=info->dlpi_name; return 1;
    }
    return 0;
}

static bool find_module(const char* name,uintptr_t* base,const char** path){
    FindModuleCtx c{name,0,nullptr}; dl_iterate_phdr(find_module_cb,&c);
    if(!c.base)return false; if(base)*base=c.base; if(path)*path=c.path; return true;
}

static bool resolve_ping_encoder(){
    if(g_ping_encoder_checked)return g_ping_encoder_ok;
    g_ping_encoder_checked=true;

    const char* path=nullptr;
    if(!find_module("net.so",&g_net_base,&path))return false;

    void* handle=dlopen(path&&*path?path:"net.so",RTLD_NOW|RTLD_NOLOAD);
    if(!handle)return false;

    using GetVer=void (*)(char*,unsigned);
    GetVer getver=(GetVer)dlsym(handle,"GetLibMtaVersion");
    if(!getver)return false;
    getver(g_net_version,sizeof(g_net_version));
    if(strcmp(g_net_version,"1.6.0-9.23312.0")!=0)return false;

    // Encoder offset validated against the supported Linux x64 net.so build.
    const uintptr_t rva=0x001DDAE0u;
    const unsigned char signature[]={0xF3,0x0F,0x1E,0xFA,0x41,0x57,0xBA,0x54,0x34,0x00,0x00,0x66,0x0F,0xEF,0xC0,0x41};
    unsigned char* f=(unsigned char*)(g_net_base+rva);
    if(memcmp(f,signature,sizeof(signature))!=0)return false;

    g_ping_encoder=(PingEncoder)(void*)f;
    g_ping_encoder_ok=true;
    return true;
}

static void release_gnu_string(GnuString64* s){
    if(!s||!s->ptr||s->ptr==s->storage.local)return;
    using DeleteFn=void (*)(void*);
    static DeleteFn del=(DeleteFn)dlsym(RTLD_DEFAULT,"_ZdlPv");
    if(del)del(s->ptr);
    s->ptr=s->storage.local; s->size=0; s->storage.local[0]=0;
}

static bool make_ping_status(unsigned players,unsigned char* out,unsigned capacity,unsigned* outlen){
    if(!out||!outlen||capacity<16||players>65535||!resolve_ping_encoder())return false;

    PingInputV1 in{};
    in.type=1;
    in.marker=0x00AB;
    in.players=(uint16_t)players;

    GnuString64 s{};
    s.ptr=s.storage.local;
    s.size=0;
    s.storage.local[0]=0;

    g_ping_encoder(&in,&s);
    bool ok=s.ptr&&s.size>=1&&s.size<capacity&&s.size<=31;
    if(ok){
        for(size_t i=0;i<s.size;++i)out[i]=(unsigned char)s.ptr[i];
        *outlen=(unsigned)s.size;
    }
    release_gnu_string(&s);
    return ok;
}

static unsigned fallback_name(unsigned index,char* out){
    const char prefix[]="PlayerBot_"; unsigned n=0;
    for(unsigned i=0;prefix[i];i++)out[n++]=prefix[i];
    unsigned id=index+1;
    if(id>=100)out[n++]=(char)('0'+(id/100)%10);
    out[n++]=(char)('0'+(id/10)%10);
    out[n++]=(char)('0'+id%10); out[n]=0; return n;
}

static unsigned bot_name(unsigned index,char* fallback,const char** value){
    if(index<g_name_count&&g_name_len[index]>0){*value=g_names[index];return g_name_len[index];}
    unsigned len=fallback_name(index,fallback);*value=fallback;return len;
}

static bool rewrite_eye2(const char* b,int length,char* out,int capacity,int* outlength){
    if(!b||length<25||length>1340||eye_index(b,length)!=1||capacity<length+300)return false;

    int p=4,offs[6],sz[6];
    for(int k=0;k<6;k++){
        if(p>=length)return false; int n=(unsigned char)b[p];
        if(n<1||p+n>length)return false; offs[k]=p;sz[k]=n;p+=n;
    }
    if(p+4>length)return false;

    int binaryCount=p+2,binaryMax=p+3;
    int real=(unsigned char)b[binaryCount],maxp=(unsigned char)b[binaryMax];
    if(maxp<1||maxp>255||real>maxp)return false;

    int pos=p+4,listed=0;
    while(pos<length){int n=(unsigned char)b[pos];if(n<1||pos+n>length)return false;pos+=n;++listed;}
    if(pos!=length||listed>real+1)return false;

    int mapstart=offs[4]+1,mapend=offs[4]+sz[4],zero=-1;
    for(int i=mapstart;i<mapend;i++)if(!b[i]){zero=i;break;}
    if(zero<0)return false;

    int cntstart=zero+1,cntend=cntstart;
    while(cntend<mapend&&b[cntend]&&b[cntend]!='/')cntend++;
    if(cntend==cntstart||cntend>=mapend||b[cntend]!='/')return false;
    int asciiCount=0;
    for(int k=cntstart;k<cntend;k++){if(b[k]<'0'||b[k]>'9')return false;asciiCount=asciiCount*10+b[k]-'0';}
    if(asciiCount!=real)return false;

    int maxstart=cntend+1,maxend=maxstart,asciiMax=0;
    while(maxend<mapend&&b[maxend]>='0'&&b[maxend]<='9'){asciiMax=asciiMax*10+b[maxend]-'0';maxend++;}
    if(maxend==maxstart||asciiMax!=maxp||maxend>=mapend||b[maxend]!=0)return false;

    int buildTypeStart=maxend+1,buildTypeEnd=buildTypeStart;
    while(buildTypeEnd<mapend&&b[buildTypeEnd])buildTypeEnd++;
    if(buildTypeEnd>=mapend)return false;
    int buildNumStart=buildTypeEnd+1,buildNumEnd=buildNumStart;
    while(buildNumEnd<mapend&&b[buildNumEnd])buildNumEnd++;
    if(buildNumEnd>=mapend)return false;
    int pingStart=buildNumEnd+1,pingEnd=pingStart;
    while(pingEnd<mapend&&b[pingEnd])pingEnd++;
    if(pingEnd>=mapend)return false;

    int total=real+(int)g_target; if(total>maxp)total=maxp;
    int extra=total-real; if(extra<1)return false;

    unsigned char newPing[32]; unsigned newPingLen=0;
    if(!make_ping_status((unsigned)total,newPing,sizeof(newPing),&newPingLen))return false;

    char digits[4],rev[4]; int dn=0,n=0,t=total;
    do{rev[dn++]=(char)('0'+t%10);t/=10;}while(t&&dn<4);
    for(int i=dn-1;i>=0;i--)digits[n++]=rev[i];

    int countDelta=n-(cntend-cntstart);
    int pingDelta=(int)newPingLen-(pingEnd-pingStart);
    int delta=countDelta+pingDelta;
    if(sz[4]+delta<1||sz[4]+delta>255)return false;

    int extraBytes=0;
    for(int i=0;i<extra;i++){
        char fallback[16];const char* name=nullptr;unsigned nameLen=bot_name((unsigned)i,fallback,&name);
        if(nameLen<1||nameLen>250)return false;extraBytes+=(int)(1+nameLen);
    }
    if(length+delta+extraBytes>capacity||length+delta+extraBytes>1340)return false;

    int j=0;
    for(int k=0;k<length;k++){
        if(k==cntstart)for(int z=0;z<n;z++)out[j++]=digits[z];
        if(k>=cntstart&&k<cntend)continue;
        if(k==pingStart)for(unsigned z=0;z<newPingLen;z++)out[j++]=(char)newPing[z];
        if(k>=pingStart&&k<pingEnd)continue;
        out[j++]=b[k];
    }
    if(j!=length+delta)return false;

    out[offs[4]]=(char)(sz[4]+delta);
    out[binaryCount+delta]=(char)total;

    for(int i=0;i<extra;i++){
        char fallback[16];const char* name=nullptr;unsigned nameLen=bot_name((unsigned)i,fallback,&name);
        out[j++]=(char)(nameLen+1);for(unsigned c=0;c<nameLen;c++)out[j++]=name[c];
    }
    if(j!=length+delta+extraBytes)return false;
    *outlength=j;return true;
}

static bool parse_and_rewrite(const char* b,int length,char* out,int capacity,int* outlength){
    if(!b||length<15||length>1400||capacity<length+5||eye_index(b,length)<0)return false;
    int offsets[9],sz[9],p=4;
    for(int i=0;i<9;i++){if(p>=length)return false;int n=(unsigned char)b[p];if(n<1||p+n>length)return false;offsets[i]=p;sz[i]=n;p+=n;}
    if(sz[7]<2||sz[7]>4||sz[8]<2||sz[8]>4)return false;
    int real=0,maxp=0;
    for(int i=offsets[7]+1;i<offsets[7]+sz[7];i++){if(b[i]<'0'||b[i]>'9')return false;real=real*10+b[i]-'0';}
    for(int i=offsets[8]+1;i<offsets[8]+sz[8];i++){if(b[i]<'0'||b[i]>'9')return false;maxp=maxp*10+b[i]-'0';}
    if(maxp<1||maxp>512||real>maxp)return false;
    int total=real+(int)g_target;if(total>maxp)total=maxp;
    char digits[4];int dn=0;do{digits[dn++]=(char)('0'+total%10);total/=10;}while(total&&dn<4);
    int nn=dn+1,delta=nn-sz[7];if(length+delta>capacity)return false;
    for(int i=0;i<offsets[7];i++)out[i]=b[i];out[offsets[7]]=(char)nn;
    for(int i=0;i<dn;i++)out[offsets[7]+1+i]=digits[dn-i-1];
    for(int i=offsets[7]+sz[7];i<length;i++)out[i+delta]=b[i];
    *outlength=length+delta;return true;
}

static ssize_t hooked_sendto(int sock,const void* buffer,size_t size,int flags,const struct sockaddr* addr,socklen_t addrlen){
    if(!g_original_sendto)return -1;
    const char* b=(const char*)buffer; int variant=eye_index(b,(int)size);
    if(g_enabled&&g_target&&variant>=0){
        char replacement[1508];int n=0;
        if((variant==1&&rewrite_eye2(b,(int)size,replacement,sizeof(replacement),&n))||
           (variant!=1&&parse_and_rewrite(b,(int)size,replacement,sizeof(replacement),&n)))
            return g_original_sendto(sock,replacement,(size_t)n,flags,addr,addrlen);
    }
    return g_original_sendto(sock,buffer,size,flags,addr,addrlen);
}

static int prot_from_phdr(const dl_phdr_info* info,uintptr_t addr){
    for(int i=0;i<info->dlpi_phnum;i++){
        const ElfW(Phdr)& p=info->dlpi_phdr[i]; if(p.p_type!=PT_LOAD)continue;
        uintptr_t lo=(uintptr_t)info->dlpi_addr+p.p_vaddr,hi=lo+p.p_memsz;
        if(addr>=lo&&addr<hi){int prot=0;if(p.p_flags&PF_R)prot|=PROT_READ;if(p.p_flags&PF_W)prot|=PROT_WRITE;if(p.p_flags&PF_X)prot|=PROT_EXEC;return prot;}
    }
    return PROT_READ;
}

static bool patch_slot(void** slot,void* replacement,int originalProt){
    if(g_patch_count>=16)return false;
    long ps=sysconf(_SC_PAGESIZE);uintptr_t page=(uintptr_t)slot&~((uintptr_t)ps-1);
    if(mprotect((void*)page,(size_t)ps,originalProt|PROT_WRITE)!=0)return false;
    g_patches[g_patch_count++]={slot,*slot,originalProt};
    *slot=replacement;
    __sync_synchronize();
    mprotect((void*)page,(size_t)ps,originalProt);
    return true;
}

static int patch_deathmatch_cb(struct dl_phdr_info* info,size_t,void*){
    if(!info||!info->dlpi_name||strcmp(base_name(info->dlpi_name),"deathmatch.so")!=0)return 0;
    uintptr_t base=(uintptr_t)info->dlpi_addr; ElfW(Dyn)* dyn=nullptr;
    for(int i=0;i<info->dlpi_phnum;i++)if(info->dlpi_phdr[i].p_type==PT_DYNAMIC){dyn=(ElfW(Dyn)*)(base+info->dlpi_phdr[i].p_vaddr);break;}
    if(!dyn)return 1;
    ElfW(Sym)* symtab=nullptr;const char* strtab=nullptr;ElfW(Rela)* rela=nullptr;size_t relasz=0;long pltrel=0;
    for(ElfW(Dyn)* d=dyn;d->d_tag!=DT_NULL;++d){
        if(d->d_tag==DT_SYMTAB)symtab=(ElfW(Sym)*)d->d_un.d_ptr;
        else if(d->d_tag==DT_STRTAB)strtab=(const char*)d->d_un.d_ptr;
        else if(d->d_tag==DT_JMPREL)rela=(ElfW(Rela)*)d->d_un.d_ptr;
        else if(d->d_tag==DT_PLTRELSZ)relasz=(size_t)d->d_un.d_val;
        else if(d->d_tag==DT_PLTREL)pltrel=d->d_un.d_val;
    }
    if(!symtab||!strtab||!rela||!relasz||pltrel!=DT_RELA)return 1;
    size_t count=relasz/sizeof(ElfW(Rela));
    for(size_t i=0;i<count;i++){
        size_t si=ELF64_R_SYM(rela[i].r_info);const char* name=strtab+symtab[si].st_name;
        if(name&&strcmp(name,"sendto")==0){void** slot=(void**)(base+rela[i].r_offset);patch_slot(slot,(void*)&hooked_sendto,prot_from_phdr(info,(uintptr_t)slot));}
    }
    return 1;
}

static void try_hook(){
    if(g_hooked)return;++g_attempts;
    g_original_sendto=(SendTo)dlsym(RTLD_DEFAULT,"sendto"); if(!g_original_sendto)return;
    unsigned before=g_patch_count;dl_iterate_phdr(patch_deathmatch_cb,nullptr);g_hooked=(g_patch_count>before);
}

static void restore_hooks(){
    for(unsigned i=0;i<g_patch_count;i++){
        GotPatch& p=g_patches[i];long ps=sysconf(_SC_PAGESIZE);uintptr_t page=(uintptr_t)p.slot&~((uintptr_t)ps-1);
        if(mprotect((void*)page,(size_t)ps,p.prot|PROT_WRITE)==0){*p.slot=p.original;__sync_synchronize();mprotect((void*)page,(size_t)ps,p.prot);}
    }
    g_patch_count=0;g_hooked=false;
}

static int playerBotsNativeBeginCount(lua_State*){g_pending=0;g_digits=0;g_receiving=true;return 0;}
static int add_digit(unsigned d){if(!g_receiving||g_digits>=3){g_receiving=false;return 0;}g_pending=g_pending*10+d;++g_digits;return 0;}
#define DIG(N) static int playerBotsNativeDigit##N(lua_State*){return add_digit(N);}
DIG(0) DIG(1) DIG(2) DIG(3) DIG(4) DIG(5) DIG(6) DIG(7) DIG(8) DIG(9)
static int playerBotsNativeCommitCount(lua_State*){if(g_receiving&&g_digits==3&&g_pending<=512){g_target=g_pending;if(!g_target)g_enabled=false;}g_receiving=false;return 0;}

static int playerBotsNativeNamesBegin(lua_State*){g_name_count=0;g_names_receiving=true;g_name_receiving=false;return 0;}
static int playerBotsNativeNameBegin(lua_State*){if(!g_names_receiving)return 0;g_pending_name_len=0;g_pending_name[0]=0;g_half_nibble=-1;g_name_receiving=true;return 0;}
static int add_nibble(unsigned value){if(!g_names_receiving||!g_name_receiving||value>15)return 0;if(g_half_nibble<0){g_half_nibble=(int)value;return 0;}unsigned byte=((unsigned)g_half_nibble<<4)|value;g_half_nibble=-1;if(byte&&g_pending_name_len<MAX_NAME_BYTES){g_pending_name[g_pending_name_len++]=(char)byte;g_pending_name[g_pending_name_len]=0;}return 0;}
#define HEX(NAME,VALUE) static int playerBotsNativeHex##NAME(lua_State*){return add_nibble(VALUE);}
HEX(0,0) HEX(1,1) HEX(2,2) HEX(3,3) HEX(4,4) HEX(5,5) HEX(6,6) HEX(7,7) HEX(8,8) HEX(9,9) HEX(A,10) HEX(B,11) HEX(C,12) HEX(D,13) HEX(E,14) HEX(F,15)
static int playerBotsNativeNameCommit(lua_State*){if(g_names_receiving&&g_name_receiving&&g_pending_name_len>0&&g_name_count<MAX_NAMES){unsigned idx=g_name_count++;g_name_len[idx]=g_pending_name_len;for(unsigned i=0;i<g_pending_name_len;i++)g_names[idx][i]=g_pending_name[i];g_names[idx][g_pending_name_len]=0;}g_name_receiving=false;g_half_nibble=-1;return 0;}
static int playerBotsNativeNamesCommit(lua_State*){g_names_receiving=false;g_name_receiving=false;g_half_nibble=-1;return 0;}
static int playerBotsNativeASEEnable(lua_State*){if(g_hooked&&g_target&&resolve_ping_encoder())g_enabled=true;return 0;}
static int playerBotsNativeASEDisable(lua_State*){g_enabled=false;return 0;}

extern "C" __attribute__((visibility("default"))) bool InitModule(ILuaModuleManager10* m,char* name,char* author,float* version){
    g_manager=m;copy_info(name,"playerbots_native");copy_info(author,"Nexys.Tuga");if(version)*version=1.0f;
    resolve_ping_encoder();try_hook();
    if(g_manager)g_manager->Printf("[playerbots] Native Linux x64 v1.0: hook=%s encoder=%s net=%s\n",g_hooked?"SIM":"NAO",g_ping_encoder_ok?"SIM":"NAO",g_net_version[0]?g_net_version:"desconhecida");
    return true;
}

extern "C" __attribute__((visibility("default"))) void RegisterFunctions(lua_State* L){
    if(!g_manager||!L)return;bool ok=true;
#define REG(N) ok=g_manager->RegisterFunction(L,#N,N)&&ok
    REG(playerBotsNativeBeginCount);REG(playerBotsNativeCommitCount);
    REG(playerBotsNativeDigit0);REG(playerBotsNativeDigit1);REG(playerBotsNativeDigit2);REG(playerBotsNativeDigit3);REG(playerBotsNativeDigit4);REG(playerBotsNativeDigit5);REG(playerBotsNativeDigit6);REG(playerBotsNativeDigit7);REG(playerBotsNativeDigit8);REG(playerBotsNativeDigit9);
    REG(playerBotsNativeNamesBegin);REG(playerBotsNativeNameBegin);REG(playerBotsNativeNameCommit);REG(playerBotsNativeNamesCommit);
    REG(playerBotsNativeHex0);REG(playerBotsNativeHex1);REG(playerBotsNativeHex2);REG(playerBotsNativeHex3);REG(playerBotsNativeHex4);REG(playerBotsNativeHex5);REG(playerBotsNativeHex6);REG(playerBotsNativeHex7);REG(playerBotsNativeHex8);REG(playerBotsNativeHex9);REG(playerBotsNativeHexA);REG(playerBotsNativeHexB);REG(playerBotsNativeHexC);REG(playerBotsNativeHexD);REG(playerBotsNativeHexE);REG(playerBotsNativeHexF);
    REG(playerBotsNativeASEEnable);REG(playerBotsNativeASEDisable);
#undef REG
    if(g_manager&&!ok)g_manager->Printf("[playerbots] ERRO: uma ou mais funcoes nativas nao foram registadas.\n");
}
extern "C" __attribute__((visibility("default"))) bool DoPulse(){if(!g_hooked&&g_attempts<10)try_hook();return true;}
extern "C" __attribute__((visibility("default"))) bool ResourceStopped(lua_State*){g_enabled=false;return true;}
extern "C" __attribute__((visibility("default"))) bool ShutdownModule(){g_enabled=false;restore_hooks();g_manager=nullptr;return true;}
