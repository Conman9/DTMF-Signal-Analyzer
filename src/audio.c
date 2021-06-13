#include <stdio.h>

#include "audio.h"
#include "debug.h"

int audio_read_header(FILE *in, AUDIO_HEADER *hp) {
    // TO BE IMPLEMENTED
	int loop;

	/*
	*  The first field in the header is the "magic number", which must be equal
	*  to 0x2e736e64 (AUDIO_MAGIC, the four ASCII characters: ".snd").
	*/

	int magic = 0;

	for(loop = 0; loop < 3; loop++){
		int c = fgetc(in);
		if(feof(in)){
			return EOF;
		}
		magic = magic + c;
		magic = magic <<  8;
	}
	int c = fgetc(in);
	if(feof(in)) return EOF;

	magic = magic + c;
	//printf("Magic Number: %x", magic);
	//printf(" / %d", magic);
	//printf("\n");

	//Check if the magic number is correct (first four characters is .snd)
	if(magic != 0x2e736e64){
		return EOF;
	}


	/*
	*	The second field in the header is the "data offset", which is the
	*   number of bytes from the beginning of the file that the audio sample
	*   data begins.  Although as far as I know, the format specifications
 	*   require that this value must be divisible by 8, some applications
	*   ("sox", for example) violate this requirement.  For audio files that
 	*   we create, this value will always be equal to 24 (AUDIO_DATA_OFFSET),
 	*   which is the number of bytes in the header.  For audio files that we read,
 	*   we must be prepared to accept larger values and to skip over the annotation
 	*   characters in order to arrive at the start of the audio sample data.
 	*/

	int offset = 0;

	for(loop = 0; loop < 3; loop++){
		int d = fgetc(in);
		if(feof(in)) return EOF;

		offset = offset + d;
		//printf("%x", c);
		//printf("\n");
		offset = offset <<  8;
	}
	int d = fgetc(in);
	if(feof(in)) return EOF;

	offset = offset + d;
	//printf("Offset: %x", offset);
	//printf(" / %d", offset);
	//printf("\n");


	/*
	*   The third field in the header is the "data size", which is the number
 	*   of bytes of audio sample data.  For audio files that we create,
 	*   this field will contain the actual number of bytes of audio sample data.
 	*   For audio files that we read, we will ignore this field and simply read
 	*   sample data until EOF is seen.
	*/

	int size = 0;

	for(loop = 0; loop < 3; loop++){
		int e = fgetc(in);
		if(feof(in)) return EOF;

		size = size + e;
		//printf("%x", c);
		//printf("\n");
		size = size <<  8;
	}
	int e = fgetc(in);
	if(feof(in)) return EOF;

	size = size + e;
	//printf("Data size: %x",size);
	//printf(" / %d",size);
	//printf("\n");


	/*
	*   The fourth field in the header specifies the encoding used for the
 	*   audio samples.  We will only support the following value:
 	*   3  (PCM16_ENCODING, specifies 16-bit linear PCM encoding)
 	*   This corresponds to a number of bytes per sample of 2 (AUDIO_BYTES_PER_SAMPLE).
 	*/

	int encode = 0;

	for(loop = 0; loop < 3; loop++){
		int f = fgetc(in);
		if(feof(in)) return EOF;

		encode = encode + f;
		//printf("%x", c);
		//printf("\n");
		encode = encode <<  8;
	}
	int f = fgetc(in);
	if(feof(in)) return EOF;

	encode = encode + f;
	//printf("Encoding: %x",encode);
	//printf(" / %d",encode);
	//printf("\n");
	//Must be equal to 3
	if(encode != 3)
		return EOF;

 	/*
 	*   The fifth field in the header specifies the "sample rate", which is the
 	*   number of frames per second.
 	*   For us, this will always be 8000 (AUDIO_FRAME_RATE).
 	*/

	int rate = 0;

	for(loop = 0; loop < 3; loop++){
		int g = fgetc(in);
		if(feof(in)) return EOF;

		rate = rate + g;
		//printf("%x", c);
		//printf("\n");
		rate = rate <<  8;
	}
	int g = fgetc(in);
	if(feof(in)) return EOF;

	rate = rate + g;
	//printf("Sample Rate %x",rate);
	//printf(" / %d",rate);
	//printf("\n");

 	/*
 	*   The sixth field in the header specifies the number of audio channels.
 	*   For us, this will always be 1 (AUDIO_CHANNELS), indicating monaural audio.
	*/

	int channel = 0;

	for(loop = 0; loop < 3; loop++){
		int h = fgetc(in);
		if(feof(in)) return EOF;

		channel = channel + h;
		//printfchannel("%x", c);
		//printf("\n");
		channel = channel <<  8;
	}
	int h = fgetc(in);
	if(feof(in)) return EOF;

	channel = channel + h;
	//printf("Channel: %x",channel);
	//printf(" / %d",channel);
	//printf("\n");
	//must always be 1
	if(channel != 1)
		return EOF;
	/*
    printf("\n");
    printf("magic number: %d\n", hp->magic_number);
    printf("data offset: %d\n", hp->data_offset);
    printf("data size: %d\n", hp->data_size);
    printf("encoding: %d\n", hp->encoding);
    printf("sample rate: %d\n", hp->sample_rate);
    printf("channels: %d\n", hp->channels);
    */

     //offset += 4;


	hp -> magic_number = magic;
	hp -> data_offset = offset;
	hp -> data_size = size;
	hp -> encoding = encode;
	hp -> sample_rate = rate;
	hp -> channels = channel;

/*
    printf("magic number: %x\n", hp->magic_number);
    printf("data offset: %d\n", hp->data_offset);
    printf("data size: %d\n", hp->data_size);
    printf("encoding: %d\n", hp->encoding);
    printf("sample rate: %d\n", hp->sample_rate);
    printf("channels: %d\n", hp->channels);

*/
     //move pointer past annotation part
    offset = offset - 24;
    for(int counter = 0; counter < offset; counter++){
    	char z = fgetc(in);
    	if(feof(in)) return EOF;
    	//printf("%x\n", z);
    }
    return 0;
}

int audio_write_header(FILE *out, AUDIO_HEADER *hp) {

/*
	//defualt values for testing program
	hp -> magic_number = 779316836;
	hp -> data_offset = 24;
	hp -> data_size = 16000;
	hp -> encoding = 3;
	hp -> sample_rate = 8000;
	hp -> channels = 1;
*/

	//magic number
	int c = (hp -> magic_number) >> 24;
	fputc(c, out);
	c = (hp -> magic_number) << 8 >> 24;
	fputc(c, out);
	c = (hp -> magic_number) << 16 >> 24;
	fputc(c, out);
	c = (hp -> magic_number) << 24 >> 24;
	fputc(c, out);

	//data_offset
	c = (hp -> data_offset) >> 24;
	fputc(c, out);
	c = (hp -> data_offset) << 8 >> 24;
	fputc(c, out);
	c = (hp -> data_offset) << 16 >> 24;
	fputc(c, out);
	c = (hp -> data_offset) << 24 >> 24;
	fputc(c, out);

	//data_size
	c = (hp -> data_size) >> 24;
	fputc(c, out);
	c = (hp -> data_size) << 8 >> 24;
	fputc(c, out);
	c = (hp -> data_size) << 16 >> 24;
	fputc(c, out);
	c = (hp -> data_size) << 24 >> 24;
	fputc(c, out);

	//encoding
	c = (hp -> encoding) >> 24;
	fputc(c, out);
	c = (hp -> encoding) << 8 >> 24;
	fputc(c, out);
	c = (hp -> encoding) << 16 >> 24;
	fputc(c, out);
	c = (hp -> encoding) << 24 >> 24;
	fputc(c, out);

	//sample_rate
	c = (hp -> sample_rate) >> 24;
	fputc(c, out);
	c = (hp -> sample_rate) << 8 >> 24;
	fputc(c, out);
	c = (hp -> sample_rate) << 16 >> 24;
	fputc(c, out);
	c = (hp -> sample_rate) << 24 >> 24;
	fputc(c, out);

	//channels
	c = (hp -> channels) >> 24;
	fputc(c, out);
	c = (hp -> channels) << 8 >> 24;
	fputc(c, out);
	c = (hp -> channels) << 16 >> 24;
	fputc(c, out);
	c = (hp -> channels) << 24 >> 24;
	fputc(c, out);

	/*
    printf("magic number: %x\n", hp->magic_number);
    printf("data offset: %x\n", hp->data_offset);
    printf("data size: %x\n", hp->data_size);
    printf("encoding: %x\n", hp->encoding);
    printf("sample rate: %x\n", hp->sample_rate);
    printf("channels: %x\n", hp->channels);
    */

    return 0;
}

int audio_read_sample(FILE *in, int16_t *samplep) {
    // TO BE IMPLEMENTED
    int16_t sam = 0;
	int z = fgetc(in);
	if(feof(in)) return EOF;
	sam += z;
	sam = sam << 8;

	//printf("%x\n", sam);

	z = fgetc(in);
	if(feof(in)) return EOF;
	sam += z;

	*samplep = sam;	//store value into pointer

    return 0;
}

int audio_write_sample(FILE *out, int16_t sample) {
    // TO BE IMPLEMENTED
	//printf("%x\n", sample);

	int c = sample >> 8;
	fputc(c, out);
	c = sample << 8 >> 8;
	fputc(c, out);

    return 0;
}
