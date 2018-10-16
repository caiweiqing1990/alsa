#include <speex/speex.h>
#include <stdio.h>

#if 0
gcc speexdec.c -o speexdec -lspeex
./speexdec enc.raw dec.raw
#endif

int main(int argc, char **argv)
{
	FILE *fin;
	FILE *fout;
	int nbBytes;
	void *dec_state;
	void *enc_state;
	SpeexBits dec_bits;
	
	int frameEncSize;
	
	speex_bits_init(&dec_bits);
	dec_state = speex_decoder_init(&speex_nb_mode);

	int frame_size;
	speex_decoder_ctl(dec_state, SPEEX_GET_FRAME_SIZE, &frame_size);
	printf("frame_size=%d\n", frame_size);
	short dec_out_frame[frame_size];
	char dec_bytes[frame_size];
	
	fin = fopen(argv[1], "r");
	fout= fopen(argv[2], "w");
	
	SpeexBits enc_bits;
	speex_bits_init(&enc_bits);
	enc_state = speex_encoder_init(&speex_nb_mode);
	speex_bits_reset(&enc_bits);
	speex_encode_int(enc_state, dec_out_frame, &enc_bits);
	frameEncSize = speex_bits_write(&enc_bits, (char *)dec_out_frame, sizeof(dec_out_frame));
	speex_bits_destroy(&enc_bits);
	speex_encoder_destroy(enc_state);
	printf("frameEncSize=%d\n", frameEncSize);
	
	while (1) 
	{
		nbBytes = fread(dec_bytes, 1, frameEncSize, fin); 
		if(feof(fin))
		break;
		speex_bits_reset(&dec_bits);
		speex_bits_read_from(&dec_bits, dec_bytes, nbBytes);
		speex_decode_int(dec_state, &dec_bits, dec_out_frame);
		//printf("nbBytes=%d\n", nbBytes);
		fwrite(dec_out_frame, sizeof(dec_out_frame), 1, fout);
	}
	
	speex_bits_destroy(&dec_bits);
	speex_decoder_destroy(dec_state);
	
	fclose(fin);
	fclose(fout);
	return 0; 
}
