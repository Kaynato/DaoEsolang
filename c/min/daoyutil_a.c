											  int          S[]
											 ={46,        33,47,41
											,37,35,      62,61,40,60,
										   58,83,91,42  ,36,59,0,114,0,
										   119,43,0},j = 0,e=0,b=0;/***/
							#define t     typedef/*****/struct/*********/
							#define z(_, b,c)t _;t/*******/b;t/********/c;
							 #define   _(_) return/*********************/_;
							#define    l(k  ,_) if(k)/*****************/y(_)
							#define      y(   _) fputc/****DAOYU*****/(_, o);
							#define       G(r,c) case/***UTILITY****/c:_(r);
							#define        P(k,r) if(!/***********/(k))_(r);
							#include        <stdio.h>/********************/
							 #define         g/**********************/n->v
							  z(d*D,d{D        X;D Z;/***************/D h;
							   char v; D   A }_,O{_*Y;_*U}T)_*I(),*L (_*,
							   _*);T*            Q;/*********/FILE*o,*i;
								int main(        int/******/argc,char**                        argv){o
								  =stdout;       P(/*****/argc>1,0)P(                     i=fopen(argv[1
									],S+17),    23)/***/while(argc-->                  2)if(!argv[argc][0
										]-45)switch(argv[argc][1])                   {case 120:b=1;break;
										   case 100:e=1;  break;                    case 111:   P(o=fopen
												(argv[argc+2-                     1],S+19),23)}while((S[16
													 ]=fgetc                     (i))+1)if(   !e)fprintf(o,
														"%c%c"                 ,S[(S[16]&240)>>4],S[S[16]&
															15                ]);fclose(i);P(e,0)P(O(),21)
															  P              (i=fopen(     argv[1],17+S),
															   11          *2)while((S[16]=fgetc(i))+1)
																if       (e&&S[16]-10&&S[16]-13&&S[16
					 ]-                                          32     )h(S[16]);fclose(i);E(I()
					  );_(0)									  }V( _*n){if(Q->U){Q->U->A=
					  n;n->h=Q->								   U;}else Q->Y
					  =n;Q->U=n;}_*u(){_*y=Q    ->Y;if(Q->Y==Q->U){Q->U
					   =Q->Y=y->A=y->h=0;_(y)}Q->Y=y->A;y->A=Q->Y->h=
						 0;_(y)}O(){Q=0;P(Q=malloc(sizeof(T)),0)Q->U =
							Q->Y=0;_(Q)}_*a(){_*n=0;P(n=malloc		(
							   sizeof(_)),00)n->A=n->h=n->Z		   =n
								   ->X=0;g=0;_(n)}_*			  v(
																 int
																 v){
																  _*n
							   =a();P							   (n,0
							 )g=v;_(n)                              }_*
							I(){while(                               Q->
							Y-Q->U)V(  L(u(),u()));                   _(Q
			   ->Y)}_      *L(_*U,_*Y ){_*n=a();P(n,                  0)if
			  (Y->v/2||U-> v/2){g=9;n->X=Y;n->Z=U;                     _(n)
			 }else{g=Y->v<0|U->v<0?Y->v+1|U->v+1                       ?-2:-
				1:Y->v?U->v?1:5:U->v?6  :0;                           free(
					Y);free(U);_(n)}}h(                              int J
					 ){S[0]=W(J);if(!(                              S[0]+4
				  ) ){while(S[0]++)V(                              v(-1))
			 ;_(0)}for(j=1<<3;j;j>>=1)V                           (v(((j
			&S[0])!=0)));}E(_*n){if(g-9){                        fprintf
			(o,(g+1) ?((g-5)/ 2)?g+2?g?" 1 "                    :" 0 ":
				" ? ":"[]":""); if(g==6)y(33                   )}else{if
				   (n->X->v   /2   &&n->Z->                   v/2){y(40)
					E(n->X    );                            y(47)E(n->Z)
							 ;y                            (41)_(0)}y((
							n                             ->X->v+1)&&(n
						   ->                            Z->v+1)?91:40)
						  if(                          n->X->v/2){l(!(n
					->Z->v-1),47)                    E(n->X);}else if(n->
				Z->v/2){l((n->X->v),47)E(   n->Z);}y(n->X->v+1&&n->Z->v+1?93:41)
			if(n->X->v/2){l(!(n->Z->v-1),33)}else if(n->Z->v/2){l(!(n->X->v),33)}_(0)}}
			W(int J){switch(J){G(0,46)G(1,33)G(2,47)G(3,93:case 41)G(4,37)G(5,35)G(6,62)G(7,61)
			  G(8,40)G(9,60)G(10,58)G(11,83)G(12,91)G(13,42)G(14,36)G(15,59)default:_(-4)}}