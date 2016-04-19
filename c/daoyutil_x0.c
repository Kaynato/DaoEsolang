#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define i_ if (S[28])
#define TS typedef struct
#define P(k, r) if(k)return r;
#define xpc(c) fputc(c, O)

TS ___* B;
TS ___ {B X;B Z;B L;B R;char v;} _;
TS ____ {_*l; _*r;} __;

__*  D ();
_*   A ();
_*   O (int);
_*   Y(__*         );
_*   I(_*  ,_*    );
char* T(_*          );

int S[] = {46, 33, 47, 41,\
           37, 35, 62, 61,\
           40, 60, 58, 83,\
           91, 42, 36, 59,\
           00, 00, 00, 00,\
          114, 00,119, 43,\
           00, 20, 20, 00,
           00};

int main(int argc, char** argv)
{
    FILE*O=stdout;
    FILE*I=NULL;
    __*Q=NULL;
    P(argc<2,0)
    P((I = fopen(argv[1], S+20)) == NULL, 23)
    while (argc-->2)
        if (!argv[argc][0]-45)
            switch(argv[argc][1])
            {
                case 120: S[28]=1; break;
                case 100: S[19]=1; break;
                case 111: P((O=fopen(argv[argc+1],S+22))==NULL,23) break;
            }
    while ((S[17]=fgetc(I))+1)
        if (!S[19]) 
            fprintf(O,"%c%c",S[(S[17]&240)>>4],S[26]=S[S[17]&15]);
    fclose(I);
    P(!S[19],0)
    P(!(Q=D()),21)
    P(!(I=fopen(argv[1],20+S)),22)
    while((S[17]=fgetc(I))+1)if(S[19]&&S[17]-10&&S[17]-13&&S[17]-32)
    L(Q,S[17]);
    fclose(I);
    i_ putchar('\n');
    npr(O, Y(Q));
    return 0;
}

__* D()
{
    __*Q=NULL;
    P(!(Q=malloc(sizeof(__))),NULL)
    Q->r=Q->l=NULL;
    return Q;
}

K(__* q, _* n)
{
    if(q->r!=NULL){q->r->R=n;n->L=q->r;}
    else q->l=n;
    q->r=n;
    i_ printf("PUSH      %s\n",T(n));
}

_* u(__* q)
{
    _*y=q->l; if(q->l==q->r)
    {q->r=q->l=y->R=y->L=NULL;i_ printf("EMPT %s\n",T(y));return y;}
    q->l=y->R;y->R=q->l->L=NULL;i_ printf("POP  %s\n", T(y));return y;
}

_* Y(__* Q)
{
    while(Q->l-Q->r) K(Q,I(u(Q),u(Q)));
    i_ putchar('\n'); return Q->l;
}

L(__* Q, int symbol)
{
    S[0]=U(symbol);
    if (S[0]==-4) { while(S[0]++)K(Q,O(-1));return; }
    for (S[18]=1<<3;S[18];S[18]>>=1)
        K(Q,O(((S[18]&S[0])!=0)));
}

_* A()
{
    _*n=NULL;
    P(!(n=malloc(sizeof(_))),NULL)
    n->R=n->L=n->Z=n->X=NULL;
    n->v=0;
    return n;
}

_* O(int v)
{
    _*n=A();
    P(n==NULL,NULL)
    n->v=v;
    return n;
}

_* I(_*r, _*l)
{
    _*n=A();
    P(!n,NULL)
    if (l->v/2==0&&r->v/2==0)
    {
        n->v=l->v>=0&&r->v>=0?l->v==0?r->v==0?0:6:r->v==0?5:1:l->v==-1&&r->v==-1?-1:-2;
        free(l);
        free(r);
        return n;
    }
    else
    {
        n->v=9;
        n->X=l;
        n->Z=r;
        return n;
    }
}

#define xpci(k, c) if(k)xpc(c)
#define xptr(s) fprintf(O,s);return
#define nvs(n, val) (n->v==val)
#define nvn(n, val) (n->v!=val)
#define nnb(n) (n->v/2!=0)

void npr(FILE* O, _* n)
{
    i_ printf("%s : ( %s / %s )\n", T(n), T(n->X), T(n->Z));
    switch(n->v)
    {
    case -1: return;
    case 5:  xptr("[]");
    case 6:  xptr("[]!");
    case 0:  xptr(" ZERO ");
    case 1:  xptr(" UNUM ");
    case -2: xptr("  ??  ");
    case 9:
        if(nnb(n->X)&&nnb(n->Z)){xpc(40); npr(O, n->X); xpc(47); npr(O, n->Z); xpc(41); return; }
        xpc(nvs(n->X, -1) || nvs(n->Z, -1) ? 40 : 91);
        if(nnb(n->X)) { xpci(nvs(n->Z, 1), 47); npr(O, n->X); }
        else if (nnb(n->Z)){ xpci(nvn(n->X, 0), 47); npr(O, n->Z); }
        xpc(nvs(n->X,-1)||nvs(n->Z, -1) ? 41 : 93);
        if (nnb(n->X)){xpci(nvs(n->Z,1),33); }
        else if(nnb(n->Z)){xpci(nvs(n->X,0),33);}
        return;
    }
}

#define G(r,c) case c:return r;

U(int symbol)
{   switch (symbol)
    {
        G(0,46)G(1,33)G(2,47)G(3,93:case 41)
        G(4,37)G(5,35)G(6,62)G(7,61)
        G(8,40)G(9,60)G(10,58)G(11,83)
        G(12,91)G(13,42)G(14,36)G(15,59)
        default:return-4;
}   }

char* T(_* n)
{   P(!n,"NULL")
    switch(n->v)
    {
        G("ZERO",0)G("UNUM",1)G("[]  ",5)
        G("[]! ",6)G("CON ",9)G("RET ",-1)
        default: return "ERR";
}   }