
#include <unistd.h>
#include "minui.h"


static void gr_flip_test();
static void gr_blit_test();


int main()
{
	gr_blit_test();
	return 0;
}


static void gr_blit_test()
{
	GRSurface *source;
	gr_init();
	/* first frame = source frame*/
	gr_color(0xFF, 0x00, 0x00, 0xFF);
	gr_fill(0, 0, 320, 240);
	gr_color(0x00, 0x00, 0xFF, 0xFF);
	gr_fill(320, 240, 640, 480);
	gr_color(0x00, 0xFF, 0x00, 0xFF);
	gr_fill(320, 0, 640, 240);
	gr_color(0xFF, 0xFF, 0xFF, 0xFF);
	gr_fill(0, 240, 320, 480);
	source = gr_get_buffer();
	/* Get next frame */
	gr_flip();
	gr_color(0x00, 0x00, 0x00, 0xFF);
	gr_fill(0, 0, 640, 480);
	/* second frame = destination frame */
	gr_blit(source, 160, 120, 320, 240, 160, 120);
	gr_flip();
	while(1) {
		gr_flip();
		//sleep(1);
	}
	gr_exit();
	return;		
}

static void gr_flip_test()
{
	gr_init();
	/* first frame */
	gr_color(0xFF, 0x00, 0x00, 0xFF);
	gr_fill(0, 0, 320, 240);
	gr_color(0x00, 0x00, 0xFF, 0xFF);
	gr_fill(320, 240, 640, 480);
	gr_color(0x00, 0xFF, 0x00, 0xFF);
	gr_fill(320, 0, 640, 240);
	gr_color(0xFF, 0xFF, 0xFF, 0xFF);
	gr_fill(0, 240, 320, 480);
	gr_flip();
	/* second frame */
	gr_color(0xFF, 0x00, 0x00, 0xFF);
	gr_fill(320, 240, 640, 480);
	gr_color(0x00, 0x00, 0xFF, 0xFF);
	gr_fill(0, 0, 320, 240);
	gr_color(0x00, 0xFF, 0x00, 0xFF);
	gr_fill(0, 240, 320, 480);
	gr_color(0xFF, 0xFF, 0xFF, 0xFF);
	gr_fill(320, 0, 640, 240);
	gr_flip();

	while(1) {
		gr_flip();
		//sleep(1);
	}
	gr_exit();
	return;	
}
