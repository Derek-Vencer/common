
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ssc.h"


#define MAX_PACKET 500


#define SSC_WAV_WRITE

#if defined(SSC_WAV_WRITE) || defined(SSC_WAV_READ)
#include "audio_wav.h"
#endif

#define TEST_DB_VAR_SEQ
#ifdef TEST_DB_VAR_SEQ
int		main_arg(int argc, char* argv[]);

int count=0;



void main(int argc, char *argv[])
{
	int		argc_t = 6;
	char	*argv_t[6];
	char	argv0[] = "HQ_BLE_Enc_Test.exe";
	char	argv1[320];
	char	argv2[320];
	char    argv3[10];
	char    argv4[10];

	argv_t[0]	= argv0;
	argv_t[1]	= "44100"; //sampling-rate

	argv_t[4]	= argv1;
	argv_t[5]	= argv2;


	strcpy(argv1, "..\\..\\44kHz_test_concat_stereo_128kbps.sec");
	strcpy(argv2, "..\\..\\44kHz_test_concat_stereo_128kbps_dec4.wav");


	main_arg(argc_t, argv_t);
}


int		main_arg(int argc, char* argv[])
#else
int main(int argc, char *argv[])
#endif
{
    int err;
    char *inFile, *outFile; 
    FILE *fin, *fout; 
    SSCDecoder *dec=NULL;
    int args;
    int len;
    int frame_size, channels;
    unsigned char *data;
    int sampling_rate;
    int max_payload_bytes = MAX_PACKET;
	  int extends_flag;
    int skip=0;
    short *out;
	int max_frame_size = 864;
	int bitrate_bps;
	int hw_tmp;


#ifdef SSC_WAV_WRITE
    WAV_HEADER		w_wav_hdr;
#endif



    inFile = argv[argc-2];
    args = 1;

    sampling_rate = (int)atol(argv[args]);
    args++;
	bitrate_bps = (int)atol(argv[args]);
	args++;
	channels = 2;
	args++;
	frame_size = 864;


	fin = fopen(inFile, "rb");
    if (!fin)
    {
        return EXIT_FAILURE;
    }

    outFile = argv[argc-1];
    fout =wav_open(outFile, &w_wav_hdr, WAV_WRITE);
    if (!fout)
    {
        fclose(fin);
        return EXIT_FAILURE;
    }




	skip = 108; 
	
	dec = (SSCDecoder *)malloc(ssc_decoder_get_size(channels));


	if (dec == NULL)
	{
		fclose(fin);
		fclose(fout);
		return EXIT_FAILURE;
	}

	err = ssc_decoder_create(44100, channels,dec);

    if (err != 0)
    {
	   free(dec);	

       fclose(fin);
       fclose(fout);
       return EXIT_FAILURE;
    }


    out = (short*)malloc(max_frame_size*channels*sizeof(short));
    data = (unsigned char*)calloc(max_payload_bytes,sizeof(char));


    while (1)
    {
		int output_samples;

		err = fread(data, 1, 4, fin);		
		switch ((data[3]&0xf7))

		{
		case 0xF0:
			bitrate_bps = 88000;
			break;
		case 0xF1:
			bitrate_bps = 96000;
			break;
		case 0xF2:
			bitrate_bps = 128000;
			break;
		case 0xF3:
			bitrate_bps = 192000;
			break;

		default:
			bitrate_bps = 88000;
			break;
		}


		len = bitrate_bps*frame_size/44100/8;
		hw_tmp = (len*3)>>7;
		len = hw_tmp+len;
		len = len+((len & 1)^1); 
			
		err = fread(data+4, 1, len-1, fin);

		if (err<len-1)
		{
            break;
        }

		output_samples = ssc_decode(dec, data, out, frame_size);


       if (output_samples>0)
       {
           if (output_samples>skip) {

			   fwrite(out+skip*channels, sizeof(short), (output_samples-skip)*channels, fout);

           }
		   skip = 0;
       } else {
  /*  	   
          fprintf(stderr, "error decoding frame: %s\n",
                          ssc_strerror(output_samples));
  */       
       }

		printf ("frame decoding : %d \n",count);
        count++;
    }


	free(dec);
    free(data);
    fclose(fin);

	w_wav_hdr.sample_rate = 44100; //sampling_rate;// 	
	w_wav_hdr.channels = channels;
    w_wav_hdr.bits_per_sample = 16;
	wav_close(fout, &w_wav_hdr, WAV_WRITE);    

    free(out);
    return EXIT_SUCCESS;
}
