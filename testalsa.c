
#define ALSA_PCM_NEW_HW_PARAMS_API


#include <alsa/asoundlib.h>
#include <linux/soundcard.h>

#define RATE		44100
#define BITS		16
#define CHANNELS	2

//#define DEFAULT_CARD	"default"		//系统默认播放声卡
//#define DEFAULT_CARD	"plughw:0,0"	//声卡0
//#define DEFAULT_CARD	"plughw:1,0"	//声卡1
#define DEFAULT_CARD	"hw:1,0"	//

snd_pcm_t *open_sound_dev(unsigned int rate, int bits, int channels, snd_pcm_stream_t type)
{
	int err;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hw_params;

	if ((err = snd_pcm_open (&handle, DEFAULT_CARD, type, 0)) < 0) {
		fprintf (stderr, "cannot snd_pcm_open (%s)\n",
			 snd_strerror (err));
		return NULL;
	}
	   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		return NULL;
	}
			 
	if ((err = snd_pcm_hw_params_any (handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_set_access (handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		return NULL;
	}
	
	if(bits == 16)
	{
		if ((err = snd_pcm_hw_params_set_format (handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
			fprintf (stderr, "cannot set sample format (%s)\n",
				 snd_strerror (err));
			return NULL;
		}
	}
	else if(bits == 8)
	{
		if ((err = snd_pcm_hw_params_set_format (handle, hw_params, SND_PCM_FORMAT_S8)) < 0) {
			fprintf (stderr, "cannot set sample format (%s)\n",
				 snd_strerror (err));
			return NULL;
		}
	}
	
	if ((err = snd_pcm_hw_params_set_rate_near (handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_set_channels (handle, hw_params, channels)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	snd_pcm_uframes_t frames = 32;
	if ((err = snd_pcm_hw_params_set_period_size_near(handle, hw_params, &frames, 0)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		return NULL;
	}
	
	if ((err = snd_pcm_hw_params (handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	snd_pcm_hw_params_free (hw_params);

	return handle;
}

void close_sound_dev(snd_pcm_t *handle)
{
	snd_pcm_close (handle);
}

snd_pcm_t *open_playback(void)
{
    return open_sound_dev(RATE, BITS, CHANNELS, SND_PCM_STREAM_PLAYBACK);
}

snd_pcm_t *open_capture(void)
{
    return open_sound_dev(RATE, BITS, CHANNELS, SND_PCM_STREAM_CAPTURE);
}


int main (int argc, char *argv[])
{
	int err;
	snd_pcm_t *playback_handle=NULL;
	//snd_pcm_t *capture_handle=NULL;
    playback_handle = open_sound_dev(RATE, BITS, CHANNELS, SND_PCM_STREAM_PLAYBACK);
    if (!playback_handle)
    {
		fprintf (stderr, "cannot open for playback\n");
        return -1;
    }

    //capture_handle = open_capture();
    //if (!capture_handle)
   // {
	//	fprintf (stderr, "cannot open for capture\n");
    //    return -1;
   // }
	
	int fd_f;
	char buffer[2048];
	
	if((fd_f = open(argv[1], O_RDWR|O_CREAT, 0666)) == -1)//open a pcm audio file
	{
		perror("cannot open the sound file");
		exit(1);
	}	
	lseek(fd_f, 42, SEEK_SET);
	int frames = 128;
	int framesize = BITS / 8 * CHANNELS;
	int size = frames * framesize;
	int rc;
	
	
	int totalsize=0;
	fprintf(stderr, "size=%d framesize=%d %d\n", size, framesize, (int)time(0));
	
	int cnt=3;
	while (1) {
#if 1
		rc = read(fd_f, buffer, size);
		if (rc == 0) //没有读取到数据 
		{
			lseek(fd_f, 42, SEEK_SET);
			cnt--;
			fprintf(stderr, "end of file on input %d\n", (int)time(0));
			printf("%d\n", getchar());			
			continue;
		}
		else if (rc != size)//实际读取 的数据 小于 要读取的数据 
		{
			fprintf(stderr,"short read: read %d bytes\n", rc);
		}
		else if (rc < 0)
		{
			fprintf(stderr,"error\n");
			break;//error
		}
#else
		//if ((err = snd_pcm_readi (capture_handle, buffer, frames)) != frames) {
		//	fprintf (stderr, "read from audio interface failed (%s)\n",
		//		 snd_strerror (err));
		//	snd_pcm_prepare(capture_handle);
			//return -1;
		//}
#endif	
		
#if	1
		if ((err = snd_pcm_writei (playback_handle, buffer, rc/framesize)) != rc/framesize) {
			fprintf (stderr, "write to audio interface failed (%s)\n",
				 snd_strerror (err));
			snd_pcm_prepare(playback_handle);
			snd_pcm_writei (playback_handle, buffer, rc/framesize);
			//return -1;
		}
#else		
		rc = write(fd_f, buffer, size);
		if (rc != size)
		{
			fprintf(stderr, "short write: wrote %d bytes\n", rc);
		}
		totalsize += size;
		if(totalsize >= 10 * RATE * BITS * CHANNELS / 8)
		{
			break;
		}
		//printf("%d %d\n", totalsize);
#endif
	}

	close(fd_f);
	snd_pcm_drain(playback_handle);
	//snd_pcm_drain(capture_handle);
	snd_pcm_close (playback_handle);
	//snd_pcm_close (capture_handle);
    return 0;
}

