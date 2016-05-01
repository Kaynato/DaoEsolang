/*COMPACT_DAOYU_INTERPRETER-KAYNATO_2016*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define r return
#define B (S*8)
#define R(c) Y(c)r
#define Q(i,j,k) Y(!(i=calloc(j,k))){printf("CANNOT ALLOCATE %d BYTES: ",j*k);perror("");abort();}
#define M(l) Y(V>=l)r;
#define T typedef
#define H while
#define D p->d
#define A p->a
#define O p->n
#define Y if
#define U p->c
#define N p->l
#define J p->g
#define P p->o
#define K C->b
#define V C->e
T unsigned long L;T void v;T struct ___{struct ___*o;struct ___*c;L* d;L a;L g;L e;L l;L n;L f;L b;}__;T __*_;
#undef T
v b(_),h(_),t(_),u(_),e(_),f(_),g(_),k(_),l(_),n(_),o(_),i(_),j(_),a(_),q(_,_),T(),W(_,L,L,L),(*F[16])(_)={0,b,
h,t,u,0,e,f,g,k,l,n,o,i,j,a};char   X(),Z(_),*I;L G(_,L,L),s(L),m=1,x=0,y,z=0,S=sizeof(L);const __ w={0,0,0,1,0
,0,1,0,0,0};_ C=0,E=0;int main(int argc,char*argv[]){__ i=w;_ p=&i;I=argv[1];R(argc<2)0;H(I[x]&&I[x]-64)x++;R(!
x++)0;y=x/2;H(y/2){y/=2;z++;}y<<=z;x=x/2-y?y*2:y;A=x*8;Y(x%S)x=S;Q(p->d,x,1)x=0;H(I[x]&&I[x]-64){y=I[x];W(p,4*x
++,4,y-33?y-47?y-93?y-41?y-37?y-35?y-62?y-61?y-40?y-60?y-58?y-83?y-91?y-42?y-36?y-59?0:15:14:13:12:11:10:9:8:7:
6:5:4:3:3:2:1);}H(*I&&*I-64)I++;I=*I-64?0:I+1;q(C=p,0);free(D);r 0;}v b(_ p){x=0;M(1)R(N<2);Y(N<=B){y=N/2;W(p,O
,N,G(p,O,y)|(G(p,O+y,y)<<y));r;}H(x<N/B/2){D[x+O/B]=D[x+O/B+N/B/2];D[x+O/B+N/B/2]=D[x+O/B];x++;}}v h(_ p){Y(Z(p
)||V>3){O+=N;r;}t(p);}v t(_ p){M(7)Y(N<A){Y(!Z(p))O-=N;N*=2;r;}R(!P);E=P;E->n=E->l=1;}v u(_ p){x=O;y=0;M(5);H(x
<A){H(!G(p,x+y,4))y+=4;Y(y)W(p,x,4,(x+y<A)?G(p,x+y,4):0);x+=4;y+=4;}}v q(_ p,_ i){y=0;M(8)C=p;Y(!U){Y(!(U=calloc
(1,sizeof(__)))){printf("MEMORY_OVERFLOW\n");r;}memcpy(U,&w,sizeof(__));p->c->o=p;p->c->f=p->f+1;p->c->d=calloc
(1,S);}E=U;J=O/4;K=J;H(m&&J<A/4&&p&&E){y=C->g;z=(C->d[y*4/32]>>(32-((y*4)%32)-4))&s(4);Y(z==5)q(E,p);else Y(z)F
[z](E);J++;}Y(!i){free(U);U=0;r;}Y(!m){free(U);U=0;m=1;}C=i;E=i->c;}v e(_ p){Y(V)V--;}v f(_ p){M(5)Y(G(p,O,1)^G
(p,O+N-1,1))T();}v g(_ p){M(7)Y(N>1){N/=2;r;}R(!U);E=U;E->l=E->a;}v k(_ p){M(9)V++;C->g=K-1;}v l(_ p){M(6)y=O;H
(y<O+N){putchar(G(p,y,8));y+=8;}}v n(_ p){M(2)Y(A==1){Y(!C->o)W(C->o,C->o->g*4,4,G(p,0,1));free(D);m=D=0;r;}A/=
2;realloc(D,A<9?1:A/8);Y(N>1)g(p);Y(O+N>A)O-=A;}v o(_ p){Y(V<1){x=N;Y(x==1){R(!U);E=U;E->l=E->a;o(E);g(E);r;}Y(
x>B){y=O/B;z=y-1+x/B;H(y<z){D[y++]=0xFFFFFFFF;D[z--]=0;}}else{W(p,O,x/2,s(x));W(p,O+(x/2),x/2,~s(x));}}g(p);} v
i(_ p){M(3)Y(!(G(p,O,1)&&!G(p,O+N-1,1)))T();}v j(_ p){L*i=0;M(1)z=0;A*=2;z=(A>B?A:B)/8/S;Q(i,z,S)memcpy(i,D,z>1
?z*S/2:S);D=i;t(p);}v a(_ p){z=O;M(6);Y(N<8){W(p,O,N,X());r;}H(z<(O+N)){W(p,z,8,X());z+=8;}}char X(){r I&&*I?*I
++:(I=0);}char Z(_ p){r!(O%(N*2));}L s(L i){R(i<B)((int)1<<i)-1;r 0xFFFFFFFF;}L G(_ p,L i,L j){r(D[i/B]>>(B-j-i
%B))&s(j);}v W(_ p,L i,L j,L k){R(j>B);z=B-j-i%B;D[i/B]&=~(s(j)<<z);D[i/B]|=(k&s(j))<<z;}v T(){R(!C);C->g++;}