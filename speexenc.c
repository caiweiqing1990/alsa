#include <speex/speex.h>
#include <stdio.h>


#if 0
gcc speexenc.c -o speexenc -lspeex
./speexenc 123456.raw enc.raw
#endif

int main(int argc, char **argv) 
{
	FILE *fin;
	FILE *fout;
	char enc_byte[200];
	int nbBytes,ret;
	void *enc_state;
	SpeexBits enc_bits;
	
	speex_bits_init(&enc_bits);
	enc_state = speex_encoder_init(&speex_nb_mode);
	
	int quality = 8;
	speex_encoder_ctl(enc_state, SPEEX_SET_QUALITY, &quality);

	int frame_size;
	speex_encoder_ctl(enc_state, SPEEX_GET_FRAME_SIZE, &frame_size);
	short enc_input_frame[160];
	
	//spx_int32_t tmp=0;
	//speex_encoder_ctl(enc_state, SPEEX_SET_VAD, &tmp);
	//speex_encoder_ctl(enc_state, SPEEX_SET_DTX, &tmp);
		
	fin = fopen(argv[1], "r");
	fout= fopen(argv[2], "w");
	while (1)
	{
		ret=fread(enc_input_frame, 1, sizeof(enc_input_frame), fin);
		if(feof(fin))
		break;
		speex_bits_reset(&enc_bits);
		speex_encode_int(enc_state, enc_input_frame, &enc_bits);
		nbBytes = speex_bits_write(&enc_bits, enc_byte, ret);
		printf("nbBytes=%d\n", nbBytes);
		fwrite(enc_byte, nbBytes, 1, fout);
	}
	
	speex_bits_destroy(&enc_bits);
	speex_encoder_destroy(enc_state);
	
	fclose(fin);
	fclose(fout);
	return 0; 
}
