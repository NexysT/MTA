// PlayerBots | Nexys.Tuga | Windows x86
// Uses the same PingStatus encoder path validated against the supported Windows x86 net.dll fingerprint.
// Ajusta apenas respostas ASE EYE e nao cria ligacoes/jogadores reais.

struct lua_State;
typedef int (__cdecl *lua_CFunction)(lua_State*);
class ILuaModuleManager10 {
public:
    virtual void ErrorPrintf(const char*, ...)=0;
    virtual void DebugPrintf(lua_State*,const char*,...)=0;
    virtual void Printf(const char*,...)=0;
    virtual bool RegisterFunction(lua_State*,const char*,lua_CFunction)=0;
};

typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef void* HMODULE;
typedef void* FARPROC;
typedef int (__stdcall *SendTo)(UINT,const char*,int,int,const void*,int);

extern "C" __declspec(dllimport) HMODULE __stdcall GetModuleHandleA(const char*);
extern "C" __declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE,const char*);
extern "C" __declspec(dllimport) int __stdcall VirtualProtect(void*,unsigned int,DWORD,DWORD*);

// CRT-free: o build usa /nodefaultlib. O otimizador MSVC pode transformar
// copias manuais em chamadas a memcpy, por isso fornecemos uma implementacao local.
extern "C" void* __cdecl memcpy(void* dst,const void* src,unsigned n){
    volatile unsigned char* d=(volatile unsigned char*)dst;
    const volatile unsigned char* s=(const volatile unsigned char*)src;
    for(unsigned i=0;i<n;++i)d[i]=s[i];
    return dst;
}

static ILuaModuleManager10* g_manager=0;
static SendTo g_original_sendto=0;
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

typedef void (__cdecl *PingEncoder)(const void*,void*);
static PingEncoder g_ping_encoder=0;
static bool g_ping_encoder_checked=false,g_ping_encoder_ok=false;

#pragma pack(push,1)
struct PingInputV1 {
    BYTE type;
    BYTE pad[3];
    DWORD unused;
    WORD marker;
    WORD players;
};
#pragma pack(pop)

struct SmallStringMSVC {
    char small[16];
    unsigned size;
    unsigned capacity;
};

static int eye_index(const char* b,int n){
    return b&&n>=4&&b[0]=='E'&&b[1]=='Y'&&b[2]=='E'&&b[3]>='1'&&b[3]<='3' ? b[3]-'1' : -1;
}

static bool same_icase(const char* a,const char* b){
    if(!a||!b)return false;
    while(*a&&*b){
        char x=*a++,y=*b++;
        if(x>='A'&&x<='Z')x+=32;
        if(y>='A'&&y<='Z')y+=32;
        if(x!=y)return false;
    }
    return !*a&&!*b;
}

static void copy_info(char* dst,const char* src){
    if(!dst||!src)return;
    unsigned i=0;
    while(i<127&&src[i]){dst[i]=src[i];i++;}
    dst[i]=0;
}

static bool resolve_ping_encoder(){
    if(g_ping_encoder_checked)return g_ping_encoder_ok;
    g_ping_encoder_checked=true;

    BYTE* base=(BYTE*)GetModuleHandleA("net.dll");
    if(!base||base[0]!='M'||base[1]!='Z')return false;
    DWORD pe=*(DWORD*)(base+0x3c);
    if(pe>0x1000||*(DWORD*)(base+pe)!=0x00004550)return false;
    BYTE* opt=base+pe+24;
    if(*(WORD*)opt!=0x10b)return false;

    DWORD timestamp=*(DWORD*)(base+pe+8);
    DWORD sizeImage=*(DWORD*)(opt+56);
    DWORD checksum=*(DWORD*)(opt+64);

    // Fail closed unless the loaded net.dll matches the tested Windows x86 build.
    if(timestamp!=0x6AADC98B || sizeImage!=0x00221000 || checksum!=0x002036FC)return false;

    BYTE* f=base+0x0002FA00;
    if(f[0]!=0x55||f[1]!=0x8B||f[2]!=0xEC||f[3]!=0x6A||f[4]!=0xFF)return false;
    if(f[0x41]!=0x80||f[0x42]!=0x3F||f[0x43]!=0x02)return false;

    g_ping_encoder=(PingEncoder)(void*)f;
    g_ping_encoder_ok=true;
    return true;
}

static bool make_ping_status(unsigned players,unsigned char* out,unsigned capacity,unsigned* outlen){
    if(!out||!outlen||capacity<16||players>65535||!resolve_ping_encoder())return false;

    PingInputV1 in;
    for(unsigned i=0;i<sizeof(in);++i)((BYTE*)&in)[i]=0;
    in.type=1;
    in.marker=0x00AB;
    in.players=(WORD)players;

    SmallStringMSVC s;
    for(unsigned i=0;i<sizeof(s);++i)((BYTE*)&s)[i]=0;
    s.capacity=15;
    s.size=0;
    s.small[0]=0;

    g_ping_encoder(&in,&s);
    if(s.capacity!=15||s.size<1||s.size>15||s.size>=capacity)return false;
    for(unsigned i=0;i<s.size;++i)out[i]=(unsigned char)s.small[i];
    *outlen=s.size;
    return true;
}

static unsigned fallback_name(unsigned index,char* out){
    const char prefix[]="PlayerBot_";
    unsigned n=0;
    for(unsigned i=0;prefix[i];i++)out[n++]=prefix[i];
    unsigned id=index+1;
    if(id>=100){out[n++]=(char)('0'+(id/100)%10);}
    out[n++]=(char)('0'+(id/10)%10);
    out[n++]=(char)('0'+id%10);
    out[n]=0;
    return n;
}

static unsigned bot_name(unsigned index,char* fallback,const char** value){
    if(index<g_name_count&&g_name_len[index]>0){
        *value=g_names[index];
        return g_name_len[index];
    }
    unsigned len=fallback_name(index,fallback);
    *value=fallback;
    return len;
}

static bool rewrite_eye2(const char* b,int length,char* out,int capacity,int* outlength){
    if(!b||length<25||length>1340||eye_index(b,length)!=1||capacity<length+300)return false;

    int p=4,offs[6],sz[6];
    for(int k=0;k<6;k++){
        if(p>=length)return false;
        int n=(unsigned char)b[p];
        if(n<1||p+n>length)return false;
        offs[k]=p;sz[k]=n;p+=n;
    }
    if(p+4>length)return false;

    int binaryCount=p+2,binaryMax=p+3;
    int real=(unsigned char)b[binaryCount];
    int maxp=(unsigned char)b[binaryMax];
    if(maxp<1||maxp>255||real>maxp)return false;

    int pos=p+4,listed=0;
    while(pos<length){
        int n=(unsigned char)b[pos];
        if(n<1||pos+n>length)return false;
        pos+=n;++listed;
    }
    if(pos!=length||listed>real+1)return false;

    int mapstart=offs[4]+1,mapend=offs[4]+sz[4];
    int zero=-1;
    for(int i=mapstart;i<mapend;i++)if(!b[i]){zero=i;break;}
    if(zero<0)return false;

    int cntstart=zero+1,cntend=cntstart;
    while(cntend<mapend&&b[cntend]&&b[cntend]!='/')cntend++;
    if(cntend==cntstart||cntend>=mapend||b[cntend]!='/')return false;

    int asciiCount=0;
    for(int k=cntstart;k<cntend;k++){
        if(b[k]<'0'||b[k]>'9')return false;
        asciiCount=asciiCount*10+b[k]-'0';
    }
    if(asciiCount!=real)return false;

    int maxstart=cntend+1,maxend=maxstart,asciiMax=0;
    while(maxend<mapend&&b[maxend]>='0'&&b[maxend]<='9'){
        asciiMax=asciiMax*10+b[maxend]-'0';
        maxend++;
    }
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

    int total=real+(int)g_target;
    if(total>maxp)total=maxp;
    int extra=total-real;
    if(extra<1)return false;

    unsigned char newPing[32];
    unsigned newPingLen=0;
    if(!make_ping_status((unsigned)total,newPing,sizeof(newPing),&newPingLen))return false;

    char digits[4],rev[4];
    int dn=0,n=0,t=total;
    do{rev[dn++]=(char)('0'+t%10);t/=10;}while(t&&dn<4);
    for(int i=dn-1;i>=0;i--)digits[n++]=rev[i];

    int countDelta=n-(cntend-cntstart);
    int pingDelta=(int)newPingLen-(pingEnd-pingStart);
    int delta=countDelta+pingDelta;
    if(sz[4]+delta<1||sz[4]+delta>255)return false;

    int extraBytes=0;
    for(int i=0;i<extra;i++){
        char fallback[16];
        const char* name=0;
        unsigned nameLen=bot_name((unsigned)i,fallback,&name);
        if(nameLen<1||nameLen>250)return false;
        extraBytes+=(int)(1+nameLen);
    }
    if(length+delta+extraBytes>capacity||length+delta+extraBytes>1340)return false;

    int j=0;
    for(int k=0;k<length;k++){
        if(k==cntstart){for(int z=0;z<n;z++)out[j++]=digits[z];}
        if(k>=cntstart&&k<cntend)continue;

        if(k==pingStart){for(unsigned z=0;z<newPingLen;z++)out[j++]=(char)newPing[z];}
        if(k>=pingStart&&k<pingEnd)continue;

        out[j++]=b[k];
    }
    if(j!=length+delta)return false;

    out[offs[4]]=(char)(sz[4]+delta);
    out[binaryCount+delta]=(char)total;

    for(int i=0;i<extra;i++){
        char fallback[16];
        const char* name=0;
        unsigned nameLen=bot_name((unsigned)i,fallback,&name);
        out[j++]=(char)(nameLen+1);
        for(unsigned c=0;c<nameLen;c++)out[j++]=name[c];
    }

    if(j!=length+delta+extraBytes)return false;
    *outlength=j;
    return true;
}

static bool parse_and_rewrite(const char* b,int length,char* out,int capacity,int* outlength){
    if(!b||length<15||length>1400||capacity<length+5)return false;
    if(eye_index(b,length)<0)return false;

    int offsets[9],sz[9],p=4;
    for(int i=0;i<9;i++){
        if(p>=length)return false;
        int n=(unsigned char)b[p];
        if(n<1||p+n>length)return false;
        offsets[i]=p;sz[i]=n;p+=n;
    }
    if(sz[7]<2||sz[7]>4||sz[8]<2||sz[8]>4)return false;

    int real=0,maxp=0;
    for(int i=offsets[7]+1;i<offsets[7]+sz[7];i++){
        if(b[i]<'0'||b[i]>'9')return false;
        real=real*10+b[i]-'0';
    }
    for(int i=offsets[8]+1;i<offsets[8]+sz[8];i++){
        if(b[i]<'0'||b[i]>'9')return false;
        maxp=maxp*10+b[i]-'0';
    }
    if(maxp<1||maxp>512||real>maxp)return false;

    int total=real+(int)g_target;
    if(total>maxp)total=maxp;

    char digits[4];
    int dn=0;
    do{digits[dn++]=(char)('0'+total%10);total/=10;}while(total&&dn<4);
    int nn=dn+1,delta=nn-sz[7];
    if(length+delta>capacity)return false;

    for(int i=0;i<offsets[7];i++)out[i]=b[i];
    out[offsets[7]]=(char)nn;
    for(int i=0;i<dn;i++)out[offsets[7]+1+i]=digits[dn-i-1];
    for(int i=offsets[7]+sz[7];i<length;i++)out[i+delta]=b[i];
    *outlength=length+delta;
    return true;
}

static int __stdcall hooked_sendto(UINT sock,const char* buffer,int size,int flags,const void* addr,int addrlen){
    if(!g_original_sendto)return -1;
    const int variant=eye_index(buffer,size);

    if(g_enabled&&g_target&&variant>=0){
        char replacement[1508];
        int n=0;
        if((variant==1&&rewrite_eye2(buffer,size,replacement,sizeof(replacement),&n))||
           (variant!=1&&parse_and_rewrite(buffer,size,replacement,sizeof(replacement),&n))){
            return g_original_sendto(sock,replacement,n,flags,addr,addrlen);
        }
    }
    return g_original_sendto(sock,buffer,size,flags,addr,addrlen);
}

static bool patch_module(HMODULE module){
    if(!module||!g_original_sendto)return false;
    BYTE* base=(BYTE*)module;
    if(base[0]!='M'||base[1]!='Z')return false;
    DWORD pe=*(DWORD*)(base+0x3c);
    if(pe>0x1000||*(DWORD*)(base+pe)!=0x00004550)return false;
    BYTE* optional=base+pe+24;
    if(*(WORD*)optional!=0x10b)return false;
    DWORD imports=*(DWORD*)(optional+96+8);
    if(!imports)return false;

    BYTE* desc=base+imports;
    bool patched=false;
    for(unsigned k=0;k<256;k++,desc+=20){
        DWORD name=*(DWORD*)(desc+12),iat=*(DWORD*)(desc+16);
        if(!name||!iat)break;
        if(!same_icase((char*)(base+name),"WS2_32.dll"))continue;
        DWORD* thunks=(DWORD*)(base+iat);
        for(unsigned j=0;j<256;j++){
            if(!thunks[j])break;
            if(thunks[j]!=(DWORD)(void*)g_original_sendto)continue;
            DWORD old=0;
            if(!VirtualProtect(&thunks[j],sizeof(DWORD),0x04,&old))continue;
            thunks[j]=(DWORD)(void*)&hooked_sendto;
            DWORD unused=0;
            VirtualProtect(&thunks[j],sizeof(DWORD),old,&unused);
            patched=true;
        }
    }
    return patched;
}

static void restore_hooks(){
    if(!g_hooked)return;
    HMODULE modules[]={GetModuleHandleA("deathmatch.dll"),GetModuleHandleA("net.dll"),GetModuleHandleA("core.dll"),GetModuleHandleA("MTA Server.exe")};
    for(unsigned m=0;m<4;m++){
        BYTE* base=(BYTE*)modules[m];
        if(!base||base[0]!='M'||base[1]!='Z')continue;
        DWORD pe=*(DWORD*)(base+0x3c);
        if(pe>0x1000||*(DWORD*)(base+pe)!=0x00004550)continue;
        BYTE* opt=base+pe+24;
        if(*(WORD*)opt!=0x10b)continue;
        DWORD imports=*(DWORD*)(opt+104);
        if(!imports)continue;
        BYTE* desc=base+imports;
        for(unsigned k=0;k<256;k++,desc+=20){
            DWORD name=*(DWORD*)(desc+12),iat=*(DWORD*)(desc+16);
            if(!name||!iat)break;
            if(!same_icase((char*)(base+name),"WS2_32.dll"))continue;
            DWORD* t=(DWORD*)(base+iat);
            for(unsigned j=0;j<256&&t[j];j++){
                if(t[j]!=(DWORD)(void*)&hooked_sendto)continue;
                DWORD old=0;
                if(!VirtualProtect(&t[j],4,0x04,&old))continue;
                t[j]=(DWORD)(void*)g_original_sendto;
                DWORD unused=0;
                VirtualProtect(&t[j],4,old,&unused);
            }
        }
    }
    g_hooked=false;
}

static void try_hook(){
    if(g_hooked)return;
    ++g_attempts;
    HMODULE w=GetModuleHandleA("WS2_32.dll");
    if(!w)return;
    g_original_sendto=(SendTo)(void*)GetProcAddress(w,(const char*)20);
    if(!g_original_sendto)return;

    bool any=false;
    any=patch_module(GetModuleHandleA("deathmatch.dll"))||any;
    any=patch_module(GetModuleHandleA("net.dll"))||any;
    any=patch_module(GetModuleHandleA("core.dll"))||any;
    any=patch_module(GetModuleHandleA("MTA Server.exe"))||any;
    if(any)g_hooked=true;
}

static int __cdecl playerBotsNativeBeginCount(lua_State*){
    g_pending=0;g_digits=0;g_receiving=true;return 0;
}
static int add_digit(unsigned d){
    if(!g_receiving||g_digits>=3){g_receiving=false;return 0;}
    g_pending=g_pending*10+d;++g_digits;return 0;
}
#define DIG(N) static int __cdecl playerBotsNativeDigit##N(lua_State*){return add_digit(N);}
DIG(0) DIG(1) DIG(2) DIG(3) DIG(4) DIG(5) DIG(6) DIG(7) DIG(8) DIG(9)

static int __cdecl playerBotsNativeCommitCount(lua_State*){
    if(g_receiving&&g_digits==3&&g_pending<=512){
        g_target=g_pending;
        if(!g_target)g_enabled=false;
    }
    g_receiving=false;
    return 0;
}

static int __cdecl playerBotsNativeNamesBegin(lua_State*){
    g_name_count=0;
    g_names_receiving=true;
    g_name_receiving=false;
    return 0;
}

static int __cdecl playerBotsNativeNameBegin(lua_State*){
    if(!g_names_receiving)return 0;
    g_pending_name_len=0;
    g_pending_name[0]=0;
    g_half_nibble=-1;
    g_name_receiving=true;
    return 0;
}

static int add_nibble(unsigned value){
    if(!g_names_receiving||!g_name_receiving||value>15)return 0;
    if(g_half_nibble<0){
        g_half_nibble=(int)value;
        return 0;
    }
    unsigned byte=((unsigned)g_half_nibble<<4)|value;
    g_half_nibble=-1;
    if(byte==0)return 0;
    if(g_pending_name_len<MAX_NAME_BYTES){
        g_pending_name[g_pending_name_len++]=(char)byte;
        g_pending_name[g_pending_name_len]=0;
    }
    return 0;
}

#define HEX(NAME,VALUE) static int __cdecl playerBotsNativeHex##NAME(lua_State*){return add_nibble(VALUE);}
HEX(0,0) HEX(1,1) HEX(2,2) HEX(3,3) HEX(4,4) HEX(5,5) HEX(6,6) HEX(7,7)
HEX(8,8) HEX(9,9) HEX(A,10) HEX(B,11) HEX(C,12) HEX(D,13) HEX(E,14) HEX(F,15)

static int __cdecl playerBotsNativeNameCommit(lua_State*){
    if(g_names_receiving&&g_name_receiving&&g_pending_name_len>0&&g_name_count<MAX_NAMES){
        unsigned idx=g_name_count++;
        g_name_len[idx]=g_pending_name_len;
        for(unsigned i=0;i<g_pending_name_len;i++)g_names[idx][i]=g_pending_name[i];
        g_names[idx][g_pending_name_len]=0;
    }
    g_name_receiving=false;
    g_half_nibble=-1;
    return 0;
}

static int __cdecl playerBotsNativeNamesCommit(lua_State*){
    g_names_receiving=false;
    g_name_receiving=false;
    g_half_nibble=-1;
    return 0;
}

static int __cdecl playerBotsNativeASEEnable(lua_State*){
    if(g_hooked&&g_target&&resolve_ping_encoder())g_enabled=true;
    return 0;
}
static int __cdecl playerBotsNativeASEDisable(lua_State*){g_enabled=false;return 0;}

extern "C" __declspec(dllexport) bool __cdecl InitModule(ILuaModuleManager10* m,char* name,char* author,float* version){
    g_manager=m;
    copy_info(name,"playerbots_native");
    copy_info(author,"Nexys.Tuga");
    if(version)*version=1.0f;
    resolve_ping_encoder();
    try_hook();
    if(g_manager)g_manager->Printf("[playerbots] Native v1.0 carregado: hook=%s encoder=%s\n",g_hooked?"SIM":"NAO",g_ping_encoder_ok?"SIM":"NAO");
    return true;
}

extern "C" __declspec(dllexport) void __cdecl RegisterFunctions(lua_State* L){
    if(!g_manager||!L)return;
    bool ok=true;
#define REG(N) ok=g_manager->RegisterFunction(L,#N,N)&&ok
    REG(playerBotsNativeBeginCount);REG(playerBotsNativeCommitCount);
    REG(playerBotsNativeDigit0);REG(playerBotsNativeDigit1);REG(playerBotsNativeDigit2);REG(playerBotsNativeDigit3);REG(playerBotsNativeDigit4);
    REG(playerBotsNativeDigit5);REG(playerBotsNativeDigit6);REG(playerBotsNativeDigit7);REG(playerBotsNativeDigit8);REG(playerBotsNativeDigit9);
    REG(playerBotsNativeNamesBegin);REG(playerBotsNativeNameBegin);REG(playerBotsNativeNameCommit);REG(playerBotsNativeNamesCommit);
    REG(playerBotsNativeHex0);REG(playerBotsNativeHex1);REG(playerBotsNativeHex2);REG(playerBotsNativeHex3);
    REG(playerBotsNativeHex4);REG(playerBotsNativeHex5);REG(playerBotsNativeHex6);REG(playerBotsNativeHex7);
    REG(playerBotsNativeHex8);REG(playerBotsNativeHex9);REG(playerBotsNativeHexA);REG(playerBotsNativeHexB);
    REG(playerBotsNativeHexC);REG(playerBotsNativeHexD);REG(playerBotsNativeHexE);REG(playerBotsNativeHexF);
    REG(playerBotsNativeASEEnable);REG(playerBotsNativeASEDisable);
#undef REG
    if(g_manager&&!ok)g_manager->Printf("[playerbots] ERRO: uma ou mais funcoes nativas nao foram registadas.\n");
}

extern "C" __declspec(dllexport) bool __cdecl DoPulse(){
    if(!g_hooked&&g_attempts<10)try_hook();
    return true;
}
extern "C" __declspec(dllexport) bool __cdecl ResourceStopped(lua_State*){g_enabled=false;return true;}
extern "C" __declspec(dllexport) bool __cdecl ShutdownModule(){g_enabled=false;restore_hooks();g_manager=0;return true;}
extern "C" int _fltused=0;
