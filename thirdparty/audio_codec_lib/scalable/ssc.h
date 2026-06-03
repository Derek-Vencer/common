

#ifndef SSC_H
#define SSC_H


#ifdef __cplusplus
extern "C" {
#endif


int ssc_decoder_get_size(int channels, int sampling_rate);


int ssc_decoder_init(
	void *st_void,
	int channels,
	int Fs,
	int sbm_on_off,
	int output_bitdepth
);


int ssc_decode(
	void *st_void,
	const unsigned char *data,
	unsigned char *pcm,
	int frame_size,
	short plc_frame,
	short decoding_channel
);



#define SBM_NORMAL_SPEED 65536




int ssc_SBM_getstatus();
int ssc_set_SBMspeed_test(int low_speed, int high_speed, int num_normal);
int ssc_sbm_interface(short apply_direction, short offsets);



/////// the below is the API for debugging. later debugging purpose ///////////
int get_internal_sampling_rate();
int get_SBM_debug_info();
int get_SBM_DIST_THR_1();
int get_SBM_DIST_THR_2();
///////////////////////////////////////



#ifdef __cplusplus
}
#endif

#endif /* SSC_H */
