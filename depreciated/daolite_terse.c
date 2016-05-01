/*COMPACT DAOYU INTERPRETER, Kaynato 2016*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define q(c) y(c)R
#define __ typedef
#define y if
#define d sizeof
#define R return
#define S (v->J)
#define U (v->K)
#define W (v->F)
#define e (v->G)
#define X (v->E)
#define f (v->A)
#define Y (v->B)
#define g (a->nn)
#define j (a->ii)
#define L(l) q(j >= l)
#define t ?argv[1][l]-

__ unsigned long Q;
__ unsigned char O;
__ void V;
__ char I;
__ struct PATH{
	struct PATH*A;
	struct PATH*B;
	Q*E;
	Q F;
	Q G;
	O ii;
	Q J;
	Q K;
	Q M;
	Q nn;
}_P;
__ _P*_;
V F1(_),F2(_),F3(_),F4(_),F6(_),F7(_),F8(_),F9(_)
,FA(_),FB(_),FC(_),FD(_),FE(_),FF(_),F5(_,_);
I GI(),AN(_);
V P(),o(_,Q,Q,Q);
O GN(I);
Q n(_,Q,Q),p(Q),P2(Q);
O k=d(Q)*8,C=0;
Q D=1;
__ V(*_f)(_);
_f ff[16]={0,F1,F2,F3,F4,0,F6,F7,F8,F9,FA,FB,FC,FD,FE,FF};
_P N={0,0,0,1,0,0,1,0,0,0};
_ a=0,Z=0;
I*b=0;
int main(int argc,I*argv[]){
	I*i=argv[1];
	Q l=0;
	_P ql=N;
	_ m=&ql;
	y(argc<2){
		puts("NO INPUT RECEIVED");
		R 0;
	};
	
	while(*i&&*i-64)i++;

	y(*i==64)b=++i;
	
	while(argv[1][l]&&argv[1][l]!=64)l++;

	q(!l)0;
	l=P2((l+1)/2);
	m->F=l*8;
	y(l%d(Q))l=d(Q);

	y(!(m->E=calloc(l,1))){
		printf("Cannot allocate %d bytes: ",l);
		perror("");
		R 0;
	}

	for(l=0;argv[1][l]&&argv[1][l]-64;){
		Q x=argv[1][l++];
		o(m,4*l,4,x-33?x-47?x-93?x-41?x-37?x-35?x-62?x-61?x-40?x-60?x-58?x-83?x-91?x-42?x-36?x-59?0:15:14:13:12:11:10:9:8:7:6:5:4:3:3:2:1);
	}
	a=m;
	F5(m,0);
	free(m->E);
	m->E=0;
	R 0;
}

Q P2(Q x){Q r=x;
	Q s=0;
	while(r/2){r/=2;
		s++;
	}r<<=s;
	y(x-r)r*=2;
	R r;
}I GI
() {q(!b)0;
	q(*b)*b++;
	b=0;
	R 0;
}V F1(_ v){Q i=0;
	Q r=0;
	L(1);
	q(S==1);
	y(S<=k){Q H=S/2;
		o(v,U,S,n(v,U,H)|
			(n(v,U+H,H)<<H));
		R;
	}while(i<((S/k)/2)){r=X[(U/k)+i];
		X[(U/k)+i]=X[(U/k)+((S/k)/2)+i];
		X[(U/k)+((S/k)
			/2)+i++]=r;
	}}V F2(_ v){y(AN(v)||(j>3))U+=S;
		else F3(v);
	}V F3(_ v){L(7);
		y(S<W){y(!AN(v))U-=S;
			S*=2;
			R;

		}q(!f);
		Z=f;
		Z->J=1;
		Z->K=1;
	}V F4(_ v){Q l=U;
		L(5);
		while(l+4<W){y(!n(v,l,4)){Q r=l;
			for(;
				!n(v,r,4)&&((r
					+4)<W);
				r+=4);
				o(v,l,4,n(v,r,4));
			o(v, r, 4, 0);
		}l+=4;
	}}V F5(_ v,_ cc){Q T=0;
		L(8);
		a=v;
		y(!Y){y(!(Y=(calloc(1,d(_P))))){printf
			("FATAL ERROR: Unable to allocate memory.");
			R;
		}memcpy(Y,&N,d(_P));
		v->B->A=v;
		v->B->M=v->M+1;
		v->B->E
		=calloc(1,d(Q));
	}Z=Y;
	e=U/4;
	g=e;
	for(;
		D&&e<(W/4)&&v&&Z;
		e++){T=a->G;
		C=(a->E[(T*4)/32]>>(32-((T*4)%32)
			-4))&p(4);
	y(C==5)F5(Z,v);
	else y(C)ff[C](Z);
}y(!cc){free(Y);
	Y=0;
	R;
}y(!D){free(Y);
	Y=0;
	D=1;
}a=cc;
Z=cc
->B;
R;
}V F6(_ v){y(j>0)j--;
}V F7(_ v){L(5);
	y(n(v,U,1)^n(v,U+S-1,1))P();
}V F8(_ v){L(7);
	y(S>1){S/=2
		;
		R;
	}q(!Y);
	Z = Y;
	Z->J=Z->F;
}V F9(_ v){L(9);
	j++;
	a->G=g-1;
}V FA(_ v){long z=U;
	L(6);
	for(;
		z<(U+S);
		z+=8
		)putc(n(v,z,8),stdout);
}V FB(_ v){L(2);
	y(W==1){Q r=n(v,0,1);
		y(a->A){Q oi=a->A->G;
			o(a->A,oi*4,4,r);

		}free(X);
		X=0;
		D=0;
		R;
	}W>>=1;
	y(W<=8)realloc(X,1);
	else realloc(X,W/8);
	y(S>1)F8(v);
	y((U+S)>W)U-=W;
}V FC
(_ v){y(j<1){Q L=S;
	y(L==1){q(!Y);
		Z=Y;
		Z->J=Z->F;
		FC(Z);
		F8(Z);
		R;
	}y(L<=k){o(v,U,L>>1,p(L));
		o(v,U+(L/2)
			,L/2,~p(L));
	}else{Q r=(U/k);
		Q ri=r+(L/k)-1;
		while(r<ri){X[r++]=0xFFFFFFFF;
			X[ri--]=0;
		}}}F8(v);
	}V FD(
		_ v){L(3);
		y(!(n(v,U,1)&&!n(v,U+S-1,1)))P();
	}V FE(_ v){Q u=0;
		Q*w=0;
		L(1);
		W<<=1;
		y(W<=k)u=k/8;
		else u=W
			/8;
		u/=d(Q);
		y(!(w=calloc(u,d(Q)))){printf("Cannot allocate %d bytes: ",u*d(Q));
		perror("");
		abort();
	}
	y(u>1)memcpy(w,X,u*d(Q)/2);
	else memcpy(w,X,d(Q));
	X=w;
	F3(v);
}V FF(_ v){Q i=U;
	L(6);
	y(S<8){o(v,U,S,GI
		());
	R;
}for(;
	i<(U+S);
	i+=8)o(v,i,8,GI());
}I AN(_ v){R U%(S<<1)==0;
}Q p(Q l){q(l<k)((Q)1<<l)-1;
	else R
		0xFFFFFFFF;
}Q n(_ v,Q i,Q L){R(X[i/k]>>(k-(i%k)-L))&p(L);
}V o(_ v,Q i,Q L,Q w){Q s=k-(i%k)-L;
	y(L>k)abort();
	X[i/k]&=~(p(L)<<s);
	X[i/k]|=((w&p(L))<<s);
}V P(){q(!a);
	a->G++;
}