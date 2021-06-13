#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
/*
	FILE *fp;
	FILE *fp1;
   	fp = fopen("/home/student/cmjurgensen/hw1/rsrc/dtmf_all.au","r");
   	fp1 = fopen("/home/student/cmjurgensen/hw1/rsrc/test.txt","w");
	//uint32_t length = 4000;
	//length is -t val * 8
	block_size = 1000;
   	int x = dtmf_detect(fp, fp1);
   	fclose(fp);
*/
/*

	//PART 5 Generate
	FILE *fp;
	FILE *fp1;
   	fp = fopen("/home/student/cmjurgensen/hw1/rsrc/dtmf.txt","r");
   	//noise_file = "/home/student/cmjurgensen/hw1/rsrc/white_noise_10s.au";
   	fp1 = fopen("/home/student/cmjurgensen/hw1/rsrc/test.au","w");
	//length = -t * 8
	uint32_t length = 800;
	noise_level = 2;
	//length is -t val * 8
   	int x = dtmf_generate(fp, fp1, length);
   	fclose(fp);
   	//printf("%d\n", x);

*/

/*
	//PART FOUR STUFF
	FILE *fp;
	AUDIO_HEADER header;
	AUDIO_HEADER *hp = &header;
   	fp = fopen("/home/student/cmjurgensen/hw1/rsrc/dtmf_0_500ms.au","r");
   	audio_read_header(fp, hp);
   	int16_t sample;
	int16_t *samplep = &sample;
	int N = 1000;

	double goertzel_k0 = (697.0 / 8000.0) * N;
	double goertzel_k1 = (770.0 / 8000.0) * N;
	double goertzel_k2 = (852.0 / 8000.0) * N;
	double goertzel_k3 = (941.0 / 8000.0) * N;
	double goertzel_k4 = (1209.0 / 8000.0) * N;
	double goertzel_k5 = (1336.0 / 8000.0) * N;
	double goertzel_k6 = (1477.0 / 8000.0) * N;
	double goertzel_k7 = (1633.0 / 8000.0) * N;

    GOERTZEL_STATE g0, g1, g2, g3, g4, g5, g6, g7;
   	goertzel_init(&g0, N, goertzel_k0);
    goertzel_init(&g1, N, goertzel_k1);
   	goertzel_init(&g2, N, goertzel_k2);
    goertzel_init(&g3, N, goertzel_k3);
    goertzel_init(&g4, N, goertzel_k4);
    goertzel_init(&g5, N, goertzel_k5);
    goertzel_init(&g6, N, goertzel_k6);
    goertzel_init(&g7, N, goertzel_k7);


    for(int count = 0; count < N-1; count++){
		audio_read_sample(fp, samplep);
		double x = (double)*samplep / INT16_MAX ;
		goertzel_step(&g0, x);
		goertzel_step(&g1, x);
		goertzel_step(&g2, x);
		goertzel_step(&g3, x);
		goertzel_step(&g4, x);
		goertzel_step(&g5, x);
		goertzel_step(&g6, x);
		goertzel_step(&g7, x);

    }
    audio_read_sample(fp, samplep);

	printf("Before changing X (int): %d\n", *samplep);
	double x = ((double)*samplep) / INT16_MAX;
	printf("After Changing X (double): %f\n", x);

    double r0 = goertzel_strength(&g0, x);
    double r1 = goertzel_strength(&g1, x);
    double r2 = goertzel_strength(&g2, x);
    double r3 = goertzel_strength(&g3, x);
    double r4 = goertzel_strength(&g4, x);
    double r5 = goertzel_strength(&g5, x);
    double r6 = goertzel_strength(&g6, x);
    double r7 = goertzel_strength(&g7, x);

    printf("697 Hz:  %f\n", r0);
    printf("770 Hz:  %f\n", r1);
    printf("852 Hz:  %f\n", r2);
    printf("941 Hz:  %f\n", r3);
    printf("1209 Hz: %f\n", r4);
    printf("1336 Hz: %f\n", r5);
    printf("1477 Hz: %f\n", r6);
 	printf("1633 Hz: %f\n\n", r7);
*/
/*

	// PART TWO STUFF

	AUDIO_HEADER header;
	AUDIO_HEADER *hp = &header;

	int16_t sample = 11891;
	int16_t *samplep = &sample;

   FILE *fp;
   fp = fopen("/home/student/cmjurgensen/hw1/rsrc/dtmf_all.au","r");
   //fp = fopen("/home/student/cmjurgensen/hw1/rsrc/test.au","r");

    if (fp == NULL)
      return 0;
  	//printf("File: %s\n", "941Hz_1sec.au");
	audio_read_header(fp, hp);
	printf("%d", hp->data_size);
  	//audio_write_header(fp, hp);
  //	audio_read_sample(fp, samplep);
  	//audio_write_sample(fp, *samplep);
*/


	//PART ONE STUFF

    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);

    if(global_options & 1)
        USAGE(*argv, EXIT_SUCCESS);

    if(global_options & 2){
    	//printf("%d", audio_samples);
    	if(stdin == NULL){
    		return EXIT_FAILURE;
    	}
   		int g = dtmf_generate(stdin, stdout, audio_samples);
   		if(g != 0)
   			return EXIT_FAILURE;
    }

    if(global_options & 4){
    	int d = dtmf_detect(stdin, stdout);
    	if(d != 0)
    		return EXIT_FAILURE;
    }

    // TO BE IMPLEMENTED
    return EXIT_SUCCESS;
    //bin/dtmf -g -t 500 < rsrc/dtmf_0_500ms.txt > msc/500out.au

}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
