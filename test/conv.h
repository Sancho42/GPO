#include <iostream>

//#pragma vector always

#ifdef OP_COUNT
extern int mulc, addc;
#define UP_MUL(N) mulc += N
#define UP_ADD(N) addc += N
#else
#define UP_MUL
#define UP_ADD
#endif


// forward declarations:

template<typename T, int N> void tc4_conv(const T *a, const T *b, T *c);
template<typename T, int N> void cconv2  (const T *a, const T *b, T *c);

// simple functions:

template<typename T>
inline void output(std::ostream& out, T *a, int M) {
 for(int i = 0; i < M; i++) out << a[i] << ' '; out << "\n";
}

template<typename T, int N>
inline void conv_naive(const T *a, const T *b, T *c) {
 for(int i = 0; i < N+N; i++) c[i] = 0;
 for(int i = 0; i < N; i++) { 
 for(int j = 0; j < N; j++) 
  c[i+j] += a[i] * b[j];
 }
}

template<typename T, int N>
inline void cconv_naive(const T *a, int Na, const T *b, int Nb, T *c) {
	for(int i = 0; i < N; i++) c[i] = 0;
	for(int i = 0; i < Na; i++) {
		for(int j = 0; j < Nb; j++) {
			c[ (i+j) % N ] += a[i] * b[j];
		}
	}
}

template<typename T, int N>
inline bool cmp(const T *a, const T *b) {
 T s = 0.0;
 for(int i = 0; i < N; i++) {
  s += (a[i] - b[i]) * (a[i] - b[i]);
 }
 return s < 1.0E-12;
}

#define simple_op(name,op) \
template<typename T, int N> \
inline void name(const T *a, const T *b, T *c) { \
 UP_ADD(N); for(int i = 0; i < N; i++) { c[i] = a[i] op b[i]; } } \
template<> \
inline void name<double,1>(const double *a, const double *b, double *c) { \
 UP_ADD(1); *c = (*a) op (*b); } \
template<> \
inline void name<double,2>(const double *a, const double *b, double *c) { \
 UP_ADD(2); *c = (*a) op (*b); c[1] = a[1] op b[1]; }
// for(int i = 0; i < N; i += 8) name<T,8>(a + i, b + i, c + i); }
// name<T,N/2>(a, b, c); \
// name<T,N/2>(a + N/2, b + N/2, c + N/2); } 

simple_op(add, +);
simple_op(sub, -);

#undef  simple_op
#define simple_op(name,op) \
template<typename T, int N> \
inline void name(const T *a, T b, T *c) { \
 UP_MUL(N); for(int i = 0; i < N; i++) { c[i] = a[i] op b; } }  \
template<> \
inline void name<double,1>(const double *a, double b, double *c) { \
 UP_MUL(1); *c = (*a) op b; } \
template<> \
inline void name<double,2>(const double *a, double b, double *c) { \
 UP_MUL(2); *c = (*a) op b; c[1] = a[1] op b; }
// for(int i = 0; i < N; i += 8) name<T,8>(a + i, b + i, c + i); }
// name<T,N/2>(a, b, c); \
// name<T,N/2>(a + N/2, b + N/2, c + N/2); }

simple_op(mul, *);

#undef  simple_op

#define simple_op(name,op) \
template<typename T, int N> \
inline void name(const T *a, const T *b, T k, T *c) { \
 UP_MUL(N); UP_ADD(N); for(int i = 0; i < N; i++) { c[i] = a[i] op k * b[i]; } } \
template<> \
inline void name<double,1>(const double *a, const double *b, double k, double *c) { \
 UP_MUL(1); UP_ADD(1); *c = (*a) op k * (*b); } \
template<> \
inline void name<double,2>(const double *a, const double *b, double k, double *c) { \
 UP_MUL(2); UP_ADD(2); *c = (*a) op k * (*b); c[1] = a[1] op k * b[1]; }
// for(int i = 0; i < N; i += 8) name<T,8>(a + i, b + i, k, c + i); }
// name<T,N/2>(a, b, k, c); \
// name<T,N/2>(a + N/2, b + N/2, k, c + N/2); }
simple_op(add_ui, +);
simple_op(sub_ui, -);

#undef  simple_op

// base template for convolution operations:

template<typename T, int N>
struct base {

 // по умолчанию используются алгоритмы Toom-Cook 4 для линейной свертки 
 // и циклическая свертка на 2 элемента
 // разрешены линейные и циклические свертки на любые степени 2: 1, 2, 4, 8, 16, ...
 static inline void  conv(const T *a, const T *b, T *c) { tc4_conv<T,N>(a, b, c); }
 static inline void cconv(const T *a, const T *b, T *c) { cconv2<T,N>(a, b, c); }
 
};

// template for base 1 (partial specialization)

template<typename T>
struct base<T, 1> {

 static inline void  conv(const T *a, const T *b, T *c) { UP_MUL(1); c[0] = (*a) * (*b); c[1] = 0; }
 static inline void cconv(const T *a, const T *b, T *c) { c[0] = (*a) * (*b); c[1] = 0; }

};

// template for base 2 (partial specialization)

template<typename T>
struct base<T, 2> {

 // Karatsuba:
 static inline void  conv(const T *a, const T *b, T *c) { 
  c[0] = a[0] * b[0];
  c[2] = a[1] * b[1];
  c[1] = c[0] + c[2] - (a[0] - a[1]) * (b[0] - b[1]);
  c[3] = 0;
  UP_MUL(3);
  UP_ADD(4);
 }
 static inline void cconv(const T *a, const T *b, T *c) { cconv2(a, b, c); }

};

//
// Toom-Cook 4 functions:
//

// matrices are taken from Bodrato and Zanoni article: 
// http://bodrato.it/papers/WhatAboutToomCookMatricesOptimality.pdf

template<typename T>
inline void tc4_evaluate(const T& a0, const T& a1, const T& a2, const T& a3, T& A1, T& A2, T& A3, T& A4, T& A5) {
 
 A1 = 8 * a0 + 4 * a1 + 2 * a2 + a3;
 A2 = a0 + a1 + a2 + a3;
 A3 = a0 - a1 + a2 - a3;
 A4 = a0 + 2 * a1 + 4 * a2 + 8 * a3;
 A5 = a0 - 2 * a1 + 4 * a2 - 8 * a3;

}

template<typename T, int N>
inline void tc4_evaluate(const T *a0, const T *a1, const T *a2, const T *a3, T *A1, T *A2, T *A3, T *A4, T *A5) {

 __declspec(align(16))
 T t0[N], t1[N], t2[N];
 T a12[N], a14[N], a22[N], a24[N];
/*
 add_ui<T,N>(a3, a1, 4, t0);
 add_ui<T,N>(a2, a0, 4, t1);
 add_ui<T,N>(t0, t1, 2, A1);

 add<T,N>(a0, a2, t0);
 add<T,N>(a1, a3, t1);
 add<T,N>(t0, t1, A2);
 sub<T,N>(t0, t1, A3);
 
 add_ui<T,N>(a0, a2, 4, t0);
 add_ui<T,N>(a1, a3, 4, t1);
 mul<T,N>(t1, 2, t2);
 add<T,N>(t0, t2, A4);
 sub<T,N>(t0, t2, A5);
*/
 add<T,N>(a1, a1, a12);
 add<T,N>(a2, a2, a22);
 add<T,N>(a12, a12, a14);
 add<T,N>(a22, a22, a24);

 add<T,N>(a3, a14, t0);
 add_ui<T,N>(a22, a0, 8, t1);
 add<T,N>(t0, t1, A1);

 add<T,N>(a0, a2, t0);
 add<T,N>(a1, a3, t1);
 add<T,N>(t0, t1, A2);
 sub<T,N>(t0, t1, A3);
 
 add<T,N>(a0, a24, t0);
 add_ui<T,N>(a12, a3, 8, t1);
 add<T,N>(t0, t1, A4);
 sub<T,N>(t0, t1, A5);

}


// Операции здесь получены следующим образом:
// Поскольку матрица постсложений работает так:
// C * S = s, 
// то можно говорить, что:
// S = inv(C) * s
// Тогда каждая строка матрицы inv(C) соответствует одному элементу вектора S.
// Бодрато и Занони предлагают путем сложений, вычитаний и умножений 
// строк матрицы inv(C) прийти к единичной матрице, и таким образом получить 
// последовательность действий для перехода от вектора S к вектору s.

template<typename T>
inline void tc4_interpolate(T& S1, T& S2, T& S3, T& S4, T& S5, T& S6, T& S7) {
 S2 += S5;
 S6 = S5 - S6;
 S4 = (S3 - S4) / 2;

 S5 -= S1 + 64 * S7;
 S5 = S5 + S5 - S6;

 S3 -= S4;

 S2 -= 65 * S3;

 S3 -= S1 + S7;
 S2 += 45 * S3;
 S5 -=  8 * S3;

 S5 /= 24;
 S6 -= S2;
 S2 -= 16 * S4;

 S2 /= 18;
 S3 -= S5;

 S4 -= S2;
 S6 += 30 * S2;

 S6 /= 60;
 S2 -= S6;
}

template<typename T, int N>
inline void tc4_interpolate(T *S1, T *S2, T *S3, T *S4, T *S5, T *S6, T *S7) {

 __declspec(align(16))
 T t0[N];
 
 sub<T,N>(S3, S4, S4); // S4 = S3 - S4;
 add<T,N>(S2, S5, S2); // S2 += S5;
 sub<T,N>(S5, S6, S6); // S6 = S5 - S6;

 mul<T,N>(S4, .5, S4); // S4 /= 2
 add_ui<T,N>(S1, S7, 64, t0);
 sub<T,N>(S5, t0, S5); //  S5 -= S1 + 64 * S7;
 sub_ui<T,N>(S6, S5,  2, S5); //  S5 = S5 + S5 - S6; (получилось S5 = S6 - 2 * S5)
 add<T,N>(S1, S7, t0); // S1 + S7 (надо ниже)
 
 sub<T,N>(S3, S4, S3); // S3 -= S4;

 sub_ui<T,N>(S2, S3, 65, S2); // S2 -= 65 * S3;

 sub<T,N>(S3, t0, S3); // S3 -= S1 + S7;
 add_ui<T,N>(S2, S3, 45, S2); // S2 += 45 * S3;
 add_ui<T,N>(S5, S3,  8, S5); // S5 -=  8 * S3; (поскольку было S5 с другим знаком, делаем S5 += 8 * S3)

 mul<T,N>(S5,-1.0/24, S5); // S5 /= 24; (и меняем знак)
 sub<T,N>(S6, S2, S6); // S6 -= S2;
 sub_ui<T,N>(S2, S4, 16, S2); // S2 -= 16 * S4;

 mul<T,N>(S2, 1.0/18, S2); // S2 /= 18;
 sub<T,N>(S3, S5, S3); // S3 -= S5;

 sub<T,N>(S4, S2, S4); // S4 -= S2;
 add_ui<T,N>(S6, S2, 30, S6); // S6 += 30 * S2;

 mul<T,N>(S6, 1.0/60, S6); // S6 /= 60;
 sub<T,N>(S2, S6, S2); // S2 -= S6;
}

// Toom-Cook 4 linear convolution itself

template<typename T, int N>
void tc4_conv(const T *a0, const T *b0, T *C) {

 // temporaries:
 __declspec(align(16))
 T A[5][N/4], B[5][N/4];
 __declspec(align(16))
 T S[3*N/2];
 T *S0 = C, *S2 = C + N/2, *S4 = C + N, *S6 = S2 + N;
 T *S1 = S, *S3 = S + N/2, *S5 = S + N;

 // split:
 const T *a1 = a0 + N/4; const T *b1 = b0 + N/4; 
 const T *a2 = a0 + N/2; const T *b2 = b0 + N/2; 
 const T *a3 = a1 + N/2; const T *b3 = b1 + N/2; 

 // evaluation:
#ifdef CONV_VECTORIZE
 tc4_evaluate<T,N/4>(a0, a1, a2, a3, A[0], A[1], A[2], A[3], A[4]);
 tc4_evaluate<T,N/4>(b0, b1, b2, b3, B[0], B[1], B[2], B[3], B[4]);
#else
 for(int i = 0; i < N/4; i++) {
  tc4_evaluate(a0[i], a1[i], a2[i], a3[i], A[0][i], A[1][i], A[2][i], A[3][i], A[4][i]);
  tc4_evaluate(b0[i], b1[i], b2[i], b3[i], B[0][i], B[1][i], B[2][i], B[3][i], B[4][i]);
 }
#endif

 // recursive calls to linear convolution:
 base<T, N/4>::conv(a0,   b0,   S0);
 base<T, N/4>::conv(A[0], B[0], S1);
 base<T, N/4>::conv(A[1], B[1], S2);
 base<T, N/4>::conv(A[2], B[2], S3);
 base<T, N/4>::conv(A[3], B[3], S4);
 base<T, N/4>::conv(A[4], B[4], S5);
 base<T, N/4>::conv(a3,   b3,   S6);

// interpolation:
#ifdef CONV_VECTORIZE
 tc4_interpolate<T,N/2-1>(S0, S1, S2, S3, S4, S5, S6);
#else
 for(int i = 0; i < N/2-1; i++) {
  tc4_interpolate(S0[i], S1[i], S2[i], S3[i], S4[i], S5[i], S6[i]);
 }
#endif

 // put it all togeter:
 for(int i = 0; i < 3*N/2; i++) {
  C[N/4 + i] += S[i];
 }
 
}

//
// Circular convolution 2x2
//

template<typename T, int N>
void cconv2(const T *a0, const T *b0, T *c) {

 // temporaries:
 __declspec(align(16))
 T A0[N/2], A1[N/2], B0[N/2], B1[N/2], C0[N], C1[N];

 // split: 
 const T *a1 = a0 + N/2;
 const T *b1 = b0 + N/2;
 
 // evaluation:
 for(int i = 0; i < N/2; i++) {
  A0[i] = a0[i] + a1[i];
  A1[i] = a0[i] - a1[i];
  B0[i] = (b0[i] + b1[i])/2;
  B1[i] = (b0[i] - b1[i])/2;
 }

 // recursive calls to linear convolution:
 base<T,N/2>::conv(A0, B0, C0);
 base<T,N/2>::conv(A1, B1, C1);

 // interpolation:
 for(int i = 0; i < N/2; i++) {
  C0[i+N/2] =  C0[i] = C0[i] + C0[i+N/2];
  C1[i+N/2] =-(C1[i] = C1[i] - C1[i+N/2]);
 }

 // put it all together:
 for(int i = 0; i < N; i++) {
  c[i] = C0[i] + C1[i];
 }

}

