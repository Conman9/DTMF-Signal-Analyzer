#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "const.h"
#include "audio.h"
#include "dtmf.h"
#include "dtmf_static.h"
#include "goertzel.h"
#include "debug.h"

int calculateFr(char value);
int calculateFc(char value);
char calculateTone(int row, int col);

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 */

/**
 * DTMF generation main function.
 * DTMF events are read (in textual tab-separated format) from the specified
 * input stream and audio data of a specified duration is written to the specified
 * output stream.  The DTMF events must be non-overlapping, in increasing order of
 * start index, and must lie completely within the specified duration.
 * The sample produced at a particular index will either be zero, if the index
 * does not lie between the start and end index of one of the DTMF events, or else
 * it will be a synthesized sample of the DTMF tone corresponding to the event in
 * which the index lies.
 *
 *  @param events_in  Stream from which to read DTMF events.
 *  @param audio_out  Stream to which to write audio header and sample data.
 *  @param length  Number of audio samples to be written.
 *  @return 0 if the header and specified number of samples are written successfully,
 *  EOF otherwise.
 */
int dtmf_generate(FILE *events_in, FILE *audio_out, uint32_t length) {
    // TO BE IMPLEMENTED
	AUDIO_HEADER header;
	AUDIO_HEADER *hp = &header;

	AUDIO_HEADER noiseheader;		//read header to skip to data of noise file
	AUDIO_HEADER *np = &noiseheader;

	FILE *noisefile;
	int isNoiseFile = 0;
	//printf("%s\n", noise_file);
	if(events_in == NULL || audio_out == NULL){
		return EOF;
	}

	if(noise_file == NULL){
		isNoiseFile = 0;
	}
	else{
		noisefile = fopen(noise_file, "r");
		if(noisefile == NULL){
			return EOF;
		}
		if(audio_read_header(noisefile, np)){	//skips to data of noise file if valid
			return EOF;
		}
		isNoiseFile = 1;
	}

	//writes valid header file

	hp -> magic_number = 0x2e736e64;
	hp -> data_offset = 24;
	hp -> data_size = length*2;
	hp -> encoding = 3;
	hp -> sample_rate = 8000;
	hp -> channels = 1;

    if(audio_write_header(audio_out, hp)){
    	return EOF;
    }

	int num_tab = 0;
	int start_index = 0;
	int end_index = 0;
	int prev_end_index = 0;
	char dtmf_tone = 0;
	int i = 0;

    char *lb = line_buf;
    char *q;
    int counter = 0;

    //Loops through all lines in the txt file.
	while(fgets(line_buf, 300, events_in)){
		counter = 0; num_tab = 0; start_index = 0; end_index = 0; dtmf_tone = 0; //reset the counter value for next loop

		for(q = lb; *q != '\n'; q++){
    		char z = *(lb + counter);

    		if(z == '\t'){
    			num_tab++;
    		}

    		else if(num_tab == 0){
    			z -= 48;
    			start_index *= 10;
    			start_index += z;
	    	}

    		else if(num_tab == 1){
    			z -= 48;
    			end_index *= 10;
    			end_index += z;
    		}

    		else if(num_tab == 2){
    			dtmf_tone = z;
    		}

    		else
    			return EOF;

    		counter++;
    	}
    	//printf("%d\n", start_index);
    	//printf("%d\n", end_index);
    	//printf("%d\n", dtmf_tone);
    	//printf("numtab %d\n", num_tab);
		int row_freq = calculateFr(dtmf_tone);
    	int col_freq = calculateFc(dtmf_tone);
    	double val = 0.0;

    	int16_t nv = 0;
    	int16_t *noise_value = &nv;
    	double v = 0.0;
    	int16_t val_trunc = 0;


    	if(prev_end_index > start_index){
    		return EOF;
    	}


    	while(i < length){
    		if(i == end_index){
    			prev_end_index = end_index;
    			break;
    		}
    		if(start_index <= i && i < end_index){
    			double row_sam = cos(2.0 * M_PI * row_freq * i / 8000.0) * 0.5;
	    		double col_sam = cos(2.0 * M_PI * col_freq * i / 8000.0) * 0.5;
	    		val = row_sam + col_sam;
	    		val *= (double)INT16_MAX;
	    		//printf("val %f\n", val );
	    		//val /= 2;
    		}
    		else{
    			val = 0.0;
    		}
    		//printf("%f\n", val);
    		if(isNoiseFile == 1){
				double w = pow(10.0, noise_level / 10.0) / (1.0 + pow(10.0, noise_level / 10.0));
				int x = audio_read_sample(noisefile, noise_value);
				if(x == 0){
					v = *noise_value * w;
					val *= (1 - w);
				//	printf("W: %f\n", w);
				//	printf("Noise Value * W: %f\n", v);
				//	printf("DTMF Value * (1-W): %f\n", val);
				}
				else{
					v = 0.0;
				}
    		}
    		val_trunc = val + v;

			//printf("Value to write: %d\n\n", val_trunc);

    		audio_write_sample(audio_out, val_trunc);
    		i++;
    	}

	}
	  	int16_t nv = 0;
    	int16_t *noise_value = &nv;
    	double v = 0;
    	int16_t val_trunc = 0;

    //Fill in rest of zeroes if not done already
	while(end_index < length){
		val_trunc = 0;
		if(isNoiseFile == 1){
				double w = pow(10.0, noise_level / 10.0) / (1.0 + pow(10.0, noise_level / 10.0));
				int x = audio_read_sample(noisefile, noise_value);
				if(x == 0){
					v = *noise_value * w;
					val_trunc *= (1 - w);

				}
				else{
					v = 0.0;
				}
		}
		val_trunc = val_trunc + v;
		audio_write_sample(audio_out, val_trunc);
		end_index++;
	}

    return 0;
}




//row frequency
int calculateFr(char value){
	if(value == '1' || value == '2' || value == '3' || value == 'A')
		return 697;
	else if(value == '4' || value == '5' || value == '6' || value == 'B')
		return 770;
	else if(value == '7' || value == '8' || value == '9' || value == 'C')
		return 852;
	else if(value == '*' || value == '0' || value == '#' || value == 'D')
		return 941;
	else
		return -1;
}


//col frequency
int calculateFc(char value){
	if(value == '1' || value == '4' || value == '7' || value == '*')
		return 1209;
	else if(value == '2' || value == '5' || value == '8' || value == '0')
		return 1336;
	else if(value == '3' || value == '6' || value == '9' || value == '#')
		return 1477;
	else if(value == 'A' || value == 'B' || value == 'C' || value == 'D')
		return 1633;
	else
		return -1;
}

/**
 * DTMF detection main function.
 * This function first reads and validates an audio header from the specified input stream.
 * The value in the data size field of the header is ignored, as is any annotation data that
 * might occur after the header.
 *
 * This function then reads audio sample data from the input stream, partititions the audio samples
 * into successive blocks of block_size samples, and for each block determines whether or not
 * a DTMF tone is present in that block.  When a DTMF tone is detected in a block, the starting index
 * of that block is recorded as the beginning of a "DTMF event".  As long as the same DTMF tone is
 * present in subsequent blocks, the duration of the current DTMF event is extended.  As soon as a
 * block is encountered in which the same DTMF tone is not present, either because no DTMF tone is
 * present in that block or a different tone is present, then the starting index of that block
 * is recorded as the ending index of the current DTMF event.  If the duration of the now-completed
 * DTMF event is greater than or equal to MIN_DTMF_DURATION, then a line of text representing
 * this DTMF event in tab-separated format is emitted to the output stream. If the duration of the
 * DTMF event is less that MIN_DTMF_DURATION, then the event is discarded and nothing is emitted
 * to the output stream.  When the end of audio input is reached, then the total number of samples
 * read is used as the ending index of any current DTMF event and this final event is emitted
 * if its length is at least MIN_DTMF_DURATION.
 *
 *   @param audio_in  Input stream from which to read audio header and sample data.
 *   @param events_out  Output stream to which DTMF events are to be written.
 *   @return 0  If reading of audio and writing of DTMF events is sucessful, EOF otherwise.
 */
int dtmf_detect(FILE *audio_in, FILE *events_out) {
    // TO BE IMPLEMENTED
	AUDIO_HEADER header;
	AUDIO_HEADER *hp = &header;
   	int z = audio_read_header(audio_in, hp);
   	if(z == -1) return EOF;

   	int16_t sample;
	int16_t *samplep = &sample;
	int N = block_size;

	int start_index = 0;
	int end_index = 0;
	//int prev_start_index = 0;
	//int prev_end_index = 0;
	int counter = 0;
	//int isDetected = 0;
	char dtmf_value;
	int block_num = 0;
	int current_event_duration = 0;
	char prev_dtmf_value = 'x';
	int check  = 0;

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

    	while(1==1){

    		GOERTZEL_STATE g0 , g1, g2, g3, g4, g5, g6, g7;
   			goertzel_init(&g0, N, goertzel_k0);
    		goertzel_init(&g1, N, goertzel_k1);
   			goertzel_init(&g2, N, goertzel_k2);
    		goertzel_init(&g3, N, goertzel_k3);
    		goertzel_init(&g4, N, goertzel_k4);
    		goertzel_init(&g5, N, goertzel_k5);
    		goertzel_init(&g6, N, goertzel_k6);
    		goertzel_init(&g7, N, goertzel_k7);
    		//loop through block
		    for(int count = 0; count < N-1; count++){

				check = audio_read_sample(audio_in, samplep);
				if(check == -1) break;

				double x = (double)sample / INT16_MAX ;
				goertzel_step(&g0, x);
				goertzel_step(&g1, x);
				goertzel_step(&g2, x);
				goertzel_step(&g3, x);
				goertzel_step(&g4, x);
				goertzel_step(&g5, x);
				goertzel_step(&g6, x);
				goertzel_step(&g7, x);
				counter++;
			}
			if(check == -1){
				break;
			}
			int check = audio_read_sample(audio_in, samplep);
			if(check == -1) break;


			double x = ((double)sample) / INT16_MAX;
			//printf("Sample: %f\n", x);

		    double r0 = goertzel_strength(&g0, x);
		    double r1 = goertzel_strength(&g1, x);
		    double r2 = goertzel_strength(&g2, x);
		    double r3 = goertzel_strength(&g3, x);
		    double r4 = goertzel_strength(&g4, x);
		    double r5 = goertzel_strength(&g5, x);
		    double r6 = goertzel_strength(&g6, x);
		    double r7 = goertzel_strength(&g7, x);
/*
		    printf("697 Hz:  %f\n", r0);
		    printf("770 Hz:  %f\n", r1);
		    printf("852 Hz:  %f\n", r2);
		    printf("941 Hz:  %f\n", r3);
		    printf("1209 Hz: %f\n", r4);
		    printf("1336 Hz: %f\n", r5);
		    printf("1477 Hz: %f\n", r6);
		 	printf("1633 Hz: %f\n\n", r7);
*/
		    //Calculate strongest row
		    double strongest_row = r0; double sec_strongest_row = r0; int row_val = 697;
		    if(r1 > strongest_row) { strongest_row = r1; row_val = 770; }
		   	else if(r1 > sec_strongest_row) { sec_strongest_row = r1; }

		    if(r2 > strongest_row) { strongest_row = r2; row_val = 852; }
   		   	else if(r2 > sec_strongest_row) { sec_strongest_row = r2; }

		    if(r3 > strongest_row) { strongest_row = r3; row_val = 941; }
		   	else if(r3 > sec_strongest_row) { sec_strongest_row = r3; }

		    //Calculate strongest col
		    double strongest_col = r4; double sec_strongest_col = r4; int col_val = 1209;
		    if(r5 > strongest_col) { strongest_col = r5; col_val = 1336; }
		    else if(r5 > sec_strongest_col) {sec_strongest_col = r5;}

		    if(r6 > strongest_col) { strongest_col = r6; col_val = 1477; }
		    else if(r6 > sec_strongest_col) {sec_strongest_col = r6;}

		    if(r7> strongest_col)  { strongest_col = r7; col_val = 1633; }
		    else if(r7 > sec_strongest_col) {sec_strongest_col = r7;}

		    dtmf_value = calculateTone(row_val, col_val);
		   // printf("Strongest Row: %f\n", strongest_row);
		   // printf("Strongest Col: %f\n\n", strongest_col);

/*
		    if(strongest_row == 0 && strongest_col == 0 && current_event_duration != 0 && isDetected != 1){
		    		if(current_event_duration >= MIN_DTMF_DURATION){
						fprintf(events_out, "%d",start_index);
						fprintf(events_out, "%c", '\t');
						fprintf(events_out, "%d",end_index);
						fprintf(events_out, "%c", '\t');
						fprintf(events_out, "%c",prev_dtmf_value);
						fprintf(events_out, "%c", '\n');
					}
					start_index = end_index;//block_num * block_size;
					prev_dtmf_value = dtmf_value;
					//output to stream
					current_event_duration = 0;
		    }
		    else if(strongest_row <= 0 && strongest_col <= 0 && current_event_duration >= 0){
		    	isDetected = 1;
		    	current_event_duration++;
				end_index = start_index + ((current_event_duration+1) * block_size);
				prev_dtmf_value = dtmf_value;

		    }
*/

/*
		    if(r0 == r1 && r1 == r2 && r2 == r3 && r3 == r4 && r4 == r5 && r5 == r6 && r6 == r7){

		    		start_index = end_index;//block_num * block_size;
					prev_dtmf_value = dtmf_value;
					//output to stream
					current_event_duration = 0;
					strongest_col = 0;
					strongest_row = 0;
		    }
*/				//printf("%d\n",current_event_duration);

			//If all 3 is right, then do next step
			if((strongest_row + strongest_col >= MINUS_20DB) &&
				((strongest_row / strongest_col >= (1.0 / FOUR_DB) && strongest_row/strongest_col <= FOUR_DB)) &&
			    ((strongest_row / (r0+r1+r2+r3-strongest_row) >= SIX_DB && strongest_col / (r4+r5+r6+r7 - strongest_col) >= SIX_DB))){

				//if(isDetected == 1){
				//	start_index = end_index;//block_num * block_size;
				//	prev_dtmf_value = dtmf_value;
					//output to stream
				//	current_event_duration = 0;
				//	isDetected = 0;
				//}
				if(prev_dtmf_value != dtmf_value){
					if(current_event_duration >= MIN_DTMF_DURATION){
						fprintf(events_out, "%d",start_index);
						fprintf(events_out, "%c", '\t');
						fprintf(events_out, "%d", end_index);
						fprintf(events_out, "%c", '\t');
						fprintf(events_out, "%c", prev_dtmf_value);
						fprintf(events_out, "%c", '\n');
					}
					start_index = block_num * block_size;
					prev_dtmf_value = dtmf_value;
					//output to stream
					current_event_duration = 0;
				}
				else{
					current_event_duration++;
					end_index = start_index + ((current_event_duration+1) * block_size);
					//isDetected = 0;
				}

			}

			block_num++;

		}

		//end_index = counter + block_num;
		//printf("End: %d\n", end_index);

	//start_index = 0;
	if(current_event_duration >= MIN_DTMF_DURATION){
		fprintf(events_out, "%d",start_index);
		fprintf(events_out, "%c", '\t');
		fprintf(events_out, "%d", end_index);
		fprintf(events_out, "%c", '\t');
		fprintf(events_out, "%c",prev_dtmf_value);
		fprintf(events_out, "%c", '\n');


	}

    return 0;
}

//returns the tone of the highest value
char calculateTone(int row, int col){
	if(row == 697 && col == 1209) return '1';
	if(row == 697 && col == 1336) return '2';
	if(row == 697 && col == 1477) return '3';
	if(row == 697 && col == 1633) return 'A';
	if(row == 770 && col == 1209) return '4';
	if(row == 770 && col == 1336) return '5';
	if(row == 770 && col == 1477) return '6';
	if(row == 770 && col == 1633) return 'B';
	if(row == 852 && col == 1209) return '7';
	if(row == 852 && col == 1336) return '8';
	if(row == 852 && col == 1477) return '9';
	if(row == 852 && col == 1633) return 'C';
	if(row == 941 && col == 1209) return '*';
	if(row == 941 && col == 1336) return '0';
	if(row == 941 && col == 1477) return '#';
	if(row == 941 && col == 1633) return 'D';
	return 'x';
}


/**
*Converts a string to an integer
*
*	@param string  A Pointer to a array of chars
*	@return int   returns the string in integer form. If invalid, returns -99999
*/

int stringToInt(char *string) {
	int length = 0;
	char *x;
	int negative = 0; //false
	int ten_factor = 1;

	for(x = string; *x != '\0'; x++){
		length++;
	}

	int number = 0;

	if( *(string) == '-') {
		negative = 1;
		length--;

	}

	if(negative == 1){
		for(int x = length; x > 0; x--){
			//int placeholder = ((*(string + x)) - 48) * ten_factor;;
			if((*(string + x) > '9' || *(string + x ) < 0)){
				//printf("%d\n", (*(string + x) - 48));
				return -999999;
			}
			int placeholder = ((*(string + x)) - 48) * ten_factor;
			number += placeholder;
			ten_factor *= 10;
			//printf("%d\n", placeholder);

			//printf("%d\n\n", number);

		}
		number *= -1;
	}

	else {

		for(int x = length-1; x > -1; x--){
			//int placeholder = ((*(string + x)) - 48) * ten_factor;;
			if((*(string + x) > '9' || *(string + x ) < '0')){
				//printf("%d\n", (*(string + x) - 48));
				return -999999;
			}
			int placeholder = ((*(string + x)) - 48) * ten_factor;
			number += placeholder;
			ten_factor *= 10;
			//printf("%d\n", placeholder);

			//printf("%d\n\n", number);

		}
	}
	//printf("%d", number);


	return number;
}

/**`
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the operation mode of the program (help, generate,
 * or detect) will be recorded in the global variable `global_options`,
 * where it will be accessible elsewhere in the program.
 * Global variables `audio_samples`, `noise file`, `noise_level`, and `block_size`
 * will also be set, either to values derived from specified `-t`, `-n`, `-l` and `-b`
 * options, or else to their default values.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected program operation mode, and global variables `audio_samples`,
 * `noise file`, `noise_level`, and `block_size` to contain values derived from
 * other option settings.
 *
 *
 *
 */
int validargs(int argc, char **argv)
{
	//printf("xyz");
	//noise_file = NULL;
	//checks if first input value is a dash
	if(argc == 1) return -1;

	if(( * ( *(argv + 1) + 0 ) ) == '-') {

		//Checks for -h flag
		if((* ( *(argv + 1) + 1)) == 'h') {
			global_options = 1; //sets global options to 1 (001 in binary)
			return 0;
		}
		//Checks for -g flag, and makes sure there is nothing after -g (e.g. -g1, -gv, etc.)
		else if((* ( *(argv + 1) + 1)) == 'g' && (* ( *( argv + 1) + 2)) <= 0) {
			//////////////////////////////////////////////////////////////////////
			//default values
			audio_samples = 8000;
			noise_level = 0;

			if(argc % 2 == 1 ) return -1;
			int t_bool = 0;
			int n_bool = 0;
			int l_bool = 0;

			///////////////////////////////////////////////////////
			if(argc >= 4){
				//check for -t
				if(((* ( *( argv + 2) + 0)) == '-' && (* ( *( argv + 2) + 1)) == 't')){
					//audio_samples = t * 8. uint_max = 4294967295
					t_bool = 1;
					//int T_value = stringToInt(*( argv + 3));
					int T_value = stringToInt(*( argv + 3));
					//int T_value = stringToInt((( *( argv + 3))));
					if((T_value > 0 && T_value <= UINT32_MAX) || (((* ( *( argv + 3) + 0) == '0') && ((* ( *( argv + 3) + 1) <= 0)))))
						audio_samples = T_value * 8;
					else
						return -1;	//return error if the t value isnt in range
				}

				//check for -n
				else if(((* ( *( argv + 2) + 0)) == '-' && (* ( *( argv + 2) + 1)) == 'n')){
					n_bool = 1;
					noise_file = (( *( argv + 3)));
				}

				//check for -l
				else if(((* ( *( argv + 2) + 0)) == '-' && (* ( *( argv + 2) + 1)) == 'l')) {
					l_bool = 1;
					int L_value = stringToInt((( *( argv + 3))));
					if(L_value >= -30 && L_value <= 30)
						noise_level = L_value;
					else
						return -1; //invalid input
				}
				else return -1;

			}

			//////////////////////////////////////////////////

			if(argc >= 6){
				//check for -t
				if(((* ( *( argv + 4) + 0)) == '-' && (* ( *( argv + 4) + 1)) == 't')){
					if(t_bool == 1) return -1; //duplicate error
					t_bool = 1;
					//audio_samples = t * 8. uint_max = 4294967295
					int T_value = stringToInt((( *( argv + 5))));
					if((T_value > 0 && T_value <= UINT32_MAX) || (((* ( *( argv + 5) + 0) == '0') && ((* ( *( argv + 5) + 1) <= 0)))))
						audio_samples = T_value * 8;
					else
						return -1;	//return error if the t value isnt in range
				}

				//check for -n
				else if(((* ( *( argv + 4) + 0)) == '-' && (* ( *( argv + 4) + 1)) == 'n')) {
					if(n_bool == 1)
						return -1; //duplicate error

					n_bool = 1;
					noise_file = (( *( argv + 5)));
				}

				//check for -l
				else if(((* ( *( argv + 4) + 0)) == '-' && (* ( *( argv + 4) + 1)) == 'l')) {
					if(l_bool == 1)
						return -1; //duplicate error

					l_bool = 1;
					int L_value = stringToInt((( *( argv + 5))));
					if(L_value >= -30 && L_value <= 30)
						noise_level = L_value;
					else
						return -1; //invalid input
				}
				else return -1;
			}
			/////////////////////////////////////////////

			if(argc == 8){
				//check for -t
				if(((* ( *( argv + 6) + 0)) == '-' && (* ( *( argv + 6) + 1)) == 't')){
					if(t_bool == 1)
						return -1; //duplicate error

					t_bool = 1;
					//audio_samples = t * 8. uint_max = 4294967295
					int T_value = stringToInt((( *( argv + 7))));
					if((T_value > 0 && T_value <= UINT32_MAX) || (((* ( *( argv + 7) + 0) == '0') && ((* ( *( argv + 7) + 1) <= 0)))))
						audio_samples = T_value * 8;
					else
						return -1;	//return error if the t value isnt in range
				}

				//check for -n
				else if(((* ( *( argv + 6) + 0)) == '-' && (* ( *( argv + 6) + 1)) == 'n')) {
					if(n_bool == 1)
						return -1; //duplicate error

					n_bool = 1;
					noise_file = (( *( argv + 7)));
				}

				//check for -l
				else if(((* ( *( argv + 6) + 0)) == '-' && (* ( *( argv + 6) + 1)) == 'l')) {
					if(l_bool == 1)
						return -1; //duplicate error

					l_bool = 1;
					int L_value = stringToInt((( *( argv + 7))));
					if(L_value >= -30 && L_value <= 30)
						noise_level = L_value;
					else
						return -1; //invalid input
				}
				else return -1;
			}

			if(argc > 8) return -1;

			if(t_bool == 0)
				audio_samples = 8000;	//default value of audio sample
			if(l_bool == 0)
				noise_level = 0;	//default value of noise level

			global_options = 2;	//sets global options to 2 (010 in binary)
			return 0; //returns 0 (valid)
		}

		//Checks for -d flag, and makes sure there is nothing after -d (e.g. -d1, -dv, etc.)
		else if((* ( *(argv + 1) + 1)) == 'd' && (* ( *( argv + 1) + 2)) <= 0) {
			//////////////////////////////////////////////////////////////////////
			if(argc % 2 == 1 ) return -1;
			//default values
			audio_samples = 8000;
			noise_level = 0;
			block_size = 100;

			int t_bool = 0;
			int n_bool = 0;
			int l_bool = 0;
			int b_bool = 0;



			if(argc >= 4){
				//check for -t
				if(((* ( *( argv + 2) + 0)) == '-' && (* ( *( argv + 2) + 1)) == 't')){
					t_bool = 1;
					//audio_samples = t * 8. uint_max = 4294967295
					int T_value = stringToInt((( *( argv + 3))));
					if((T_value > 0 && T_value <= UINT32_MAX) || (((* ( *( argv + 3) + 0) == '0') && ((* ( *( argv + 3) + 1) <= 0)))))
						audio_samples = T_value * 8;
					else
						return -1;	//return error if the t value isnt in range
				}

				//check for -n
				else if(((* ( *( argv + 2) + 0)) == '-' && (* ( *( argv + 2) + 1)) == 'n')){
					n_bool = 1;
					noise_file = (( *( argv + 3)));
				}

				//check for -l
				else if(((* ( *( argv + 2) + 0)) == '-' && (* ( *( argv + 2) + 1)) == 'l')) {
					l_bool = 1;
					int L_value = stringToInt((( *( argv + 3))));
					if(L_value >= -30 && L_value <= 30)
						noise_level = L_value;
					else
						return -1; //invalid input
				}

				//check for -b
				else if(((* ( *( argv + 2) + 0)) == '-' && (* ( *( argv + 2) + 1)) == 'b')) {
					b_bool = 1;
					int B_value = stringToInt((( *( argv + 3))));
					//printf("%d\n\n", B_value);
					if(B_value >= 10 && B_value <= 1000)
						block_size = B_value;
					else
						return -1;
				}
				else return -1;

			}

			//////////////////////////////////////////////////

			if(argc >= 6){
				//check for -t
				if(((* ( *( argv + 4) + 0)) == '-' && (* ( *( argv + 4) + 1)) == 't')){
					if(t_bool == 1)
						return -1; //duplicate error

					t_bool = 1;
					//audio_samples = t * 8. uint_max = 4294967295
					int T_value = stringToInt((( *( argv + 5))));
					if((T_value > 0 && T_value <= UINT32_MAX) || (((* ( *( argv + 5) + 0) == '0') && ((* ( *( argv + 5) + 1) <= 0)))))
						audio_samples = T_value * 8;
					else
						return -1;	//return error if the t value isnt in range
				}

				//check for -n
				else if(((* ( *( argv + 4) + 0)) == '-' && (* ( *( argv + 4) + 1)) == 'n')){
					if(n_bool == 1)
						return -1; //duplicate error

					n_bool = 1;
					noise_file = (( *( argv + 5)));
				}

				//check for -l
				else if(((* ( *( argv + 4) + 0)) == '-' && (* ( *( argv + 4) + 1)) == 'l')) {
					if(l_bool == 1)
						return -1; //duplicate error

					l_bool = 1;
					int L_value = stringToInt((( *( argv + 5))));
					if(L_value >= -30 && L_value <= 30)
						noise_level = L_value;
					else
						return -1; //invalid input
				}
				else if(((* ( *( argv + 4) + 0)) == '-' && (* ( *( argv + 4) + 1)) == 'b')) {
					if(b_bool == 1)
						return -1; //duplicate error

					b_bool = 1;
					int B_value = stringToInt((( *( argv + 5))));
					if(B_value >= 10 && B_value <= 1000)
						block_size = B_value;
					else
						return -1;
				}
				else return -1;
			}
			/////////////////////////////////////////////

			if(argc >= 8){
				//check for -t
				if(((* ( *( argv + 6) + 0)) == '-' && (* ( *( argv + 6) + 1)) == 't')){
					if(t_bool == 1)
						return -1; //duplicate error

					t_bool = 1;
					//audio_samples = t * 8. uint_max = 4294967295
					int T_value = stringToInt((( *( argv + 7))));
					if((T_value > 0 && T_value <= UINT32_MAX) || (((* ( *( argv + 7) + 0) == '0') && ((* ( *( argv + 7) + 1) <= 0)))))
						audio_samples = T_value * 8;
					else
						return -1;	//return error if the t value isnt in range
				}

				//check for -n
				else if(((* ( *( argv + 6) + 0)) == '-' && (* ( *( argv + 6) + 1)) == 'n')){
					if(n_bool == 1)
						return -1; //duplicate error

					n_bool = 1;
					noise_file = (( *( argv + 7)));
				}

				//check for -l
				else if(((* ( *( argv + 6) + 0)) == '-' && (* ( *( argv + 6) + 1)) == 'l')) {
					if(l_bool == 1)
						return -1; //duplicate error
					l_bool = 1;
					int L_value = stringToInt((( *( argv + 7))));
					if(L_value >= -30 && L_value <= 30)
						noise_level = L_value;
					else
						return -1; //invalid input
				}

				//checks for -b
				else if(((* ( *( argv + 6) + 0)) == '-' && (* ( *( argv + 6) + 1)) == 'b')) {
					if(b_bool == 1)
						return -1; //duplicate error

					b_bool = 1;
					int B_value = stringToInt((( *( argv + 7))));
					if(B_value >= 10 && B_value <= 1000)
						block_size = B_value;
					else
						return -1;
				}
				else return -1;
			}


			if(argc == 10){
				//check for -t
				if(((* ( *( argv + 8) + 0)) == '-' && (* ( *( argv + 8) + 1)) == 't')){
					if(t_bool == 1)
						return -1; //duplicate error

					t_bool = 1;
					//audio_samples = t * 8. uint_max = 4294967295
					int T_value = stringToInt((( *( argv + 9))));
					if((T_value > 0 && T_value <= UINT32_MAX) || (((* ( *( argv + 9) + 0) == '0') && ((* ( *( argv + 9) + 1) <= 0)))))
						audio_samples = T_value * 8;
					else
						return -1;	//return error if the t value isnt in range
				}

				//check for -n
				else if(((* ( *( argv + 8) + 0)) == '-' && (* ( *( argv + 8) + 1)) == 'n')){
					if(n_bool == 1)
						return -1; //duplicate error

					n_bool = 1;
					noise_file = (( *( argv + 9)));
				}

				//check for -l
				else if(((* ( *( argv + 8) + 0)) == '-' && (* ( *( argv + 8) + 1)) == 'l')) {
					if(l_bool == 1)
						return -1; //duplicate error

					l_bool = 1;
					int L_value = stringToInt((( *( argv + 9))));
					if(L_value >= -30 && L_value <= 30)
						noise_level = L_value;
					else
						return -1; //invalid input
				}

				//checks for -b
				else if(((* ( *( argv + 8) + 0)) == '-' && (* ( *( argv + 8) + 1)) == 'b')) {
					if(b_bool == 1)
						return -1; //duplicate error

					b_bool = 1;
					int B_value = stringToInt((( *( argv + 9))));
					if(B_value >= 10 && B_value <= 1000)
						block_size = B_value;
					else
						return -1;
				}
				else return -1;
			}
			if(argc > 10) return -1;

			if(t_bool == 0)
				audio_samples = 8000; //default value
			if(l_bool == 0)
				noise_level = 0;	//defualt value
			if(b_bool == 0)
				block_size = 100; //defualt value

			global_options = 4;	//sets global options to 2 (010 in binary)

			return 0; //returns 0 (valid)
		}

		//no correct flag
		return -1;
	}
	//no flag provided
	else
		return -1;

}


