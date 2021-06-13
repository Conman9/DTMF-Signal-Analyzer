#include <stdint.h>
#include <math.h>

#include "debug.h"
#include "goertzel.h"

void goertzel_init(GOERTZEL_STATE *gp, uint32_t N, double k) {

	//precalculation of constants
	double A = 2.0 * M_PI * (k / N);
	double B = 2.0 * cos(A);

	//state variables
	double s0 = 0.0;
	double s1 = 0.0;
	double s2 = 0.0;

	//Assigning values into struct
	gp -> A = A;
	gp -> B = B;
	gp -> s0 = s0;
	gp -> s1 = s1;
	gp -> s2 = s2;
	gp -> N = N;
	gp -> k = k;

/*
	printf("A: %f\n", gp -> A);
	printf("B: %f\n", gp -> B);
	printf("N: %d\n", gp -> N);
	printf("k: %f\n", gp -> k);
	printf("s0: %f\n", gp -> s0);
	printf("s1: %f\n", gp -> s1);
	printf("s2: %f\n\n", gp -> s2);
*/
}

void goertzel_step(GOERTZEL_STATE *gp, double x) {
	//main loop first iteration and any iteration besides the last
	gp -> s0 = x + ((gp -> B) * (gp -> s1)) - (gp -> s2);
	gp -> s2 = gp -> s1;
	gp -> s1 = gp -> s0;
/*
	printf("A: %f\n", gp -> A);
	printf("B: %f\n", gp -> B);
	printf("N: %d\n", gp -> N);
	printf("k: %f\n", gp -> k);
	printf("s0: %f\n", gp -> s0);
	printf("s1: %f\n", gp -> s1);
	printf("s2: %f\n\n", gp -> s2);
	*/
}

double goertzel_strength(GOERTZEL_STATE *gp, double x) {
    //Finalizing Calculations
/*
	printf("A: %f\n", gp -> A);
	printf("B: %f\n", gp -> B);
	printf("N: %d\n", gp -> N);
	printf("k: %f\n", gp -> k);
	printf("s0: %f\n", gp -> s0);
	printf("s1: %f\n", gp -> s1);
	printf("s2: %f\n\n", gp -> s2);
*/

    gp -> s0 = x + ((gp -> B) * (gp -> s1)) - (gp -> s2);
    double A = gp -> A;
    double B = gp -> B;

    //C = cos(A) - j*sin(A)
    double real_C = cos(A);
    double imag_C = -1.0 * sin(A);
   // printf("A: %f\n", A);
   // printf("B: %f\n", B);
   // printf("real_c: %f\n", real_C);
   // printf("imag_c: %f\n", imag_C);


    double real_D = cos(A * (gp -> N-1.0));
    double imag_D = -1.0 * sin(A * (gp -> N-1.0));


   // printf("real_D: %f\n", real_D);
 //   printf("imag_D: %f\n", imag_D);


    double real_y = (gp -> s0) - ((gp -> s1) * real_C);
    double imag_y = (-1.0)*((gp -> s1) * imag_C);

   // printf("real_Y: %f\n", real_y);
   // printf("imag_Y: %f\n", imag_y);

    //real_y = real_y * real_D;
    //real_y = (real_y + imag_y) * (real_D + imag_D);
    double new_real_y = (real_y * real_D) - (imag_y * imag_D);

    double new_imag_y = (real_y * imag_D) + (imag_y * real_D) ;

    //printf("real_Y * real_D: %f\n", new_real_y);
    //printf("imag_Y * imag_D: %f\n\n", new_imag_y);

    double N = gp -> N;

    double actual_y = (new_real_y * new_real_y) + (new_imag_y * new_imag_y);
    // (2*|y|^2) /N^2

   // return ((2 * ((real_y * real_y) + (imag_y * imag_y))) / (N * N)) ;
    return (2.0 * actual_y) / (N * N);
}
