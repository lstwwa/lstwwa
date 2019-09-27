#include "common.h"
#include "jpeglib.h"

static int lcd;
static int lcd_w;
static int lcd_h;
static int lcd_bpp;
static int screen_size;
static struct fb_var_screeninfo vsinfo;
static char *lcdmem;

static bool first = true;

void display(char *jpg, int x, int y)
{
	// 第一次显示图片的时候，就准备好LCD设备
	if(first)
	{
		lcd = open("/dev/fb0", O_RDWR);

		bzero(&vsinfo, sizeof(vsinfo));
		ioctl(lcd, FBIOGET_VSCREENINFO, &vsinfo); // 获取了LCD的硬件参数

		lcd_w = vsinfo.xres;
		lcd_h = vsinfo.yres;
		lcd_bpp =  vsinfo.bits_per_pixel;
		screen_size = lcd_w * lcd_h * lcd_bpp/8;

		lcdmem = mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, lcd, 0);
		first = false;
	}



	int jpgfd = open(jpg, O_RDONLY);

	int jpg_size = lseek(jpgfd, 0L, SEEK_END);
	lseek(jpgfd, 0L, SEEK_SET);

	char *jpg_buffer = calloc(1, jpg_size);

	// 完整地读完了 a.jpg 文件的内容，妥妥地放到了 jpg_buffer 中
	int total = jpg_size;
	while(total > 0)
	{
		int n = read(jpgfd, jpg_buffer+(jpg_size-total), total);
		total -= n;
	}

	close(jpgfd);


    	// JPG  ==>  RGB

	// 声明解压缩结构体，以及错误管理结构体
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	// 使用缺省的出错处理来初始化解压缩结构体
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// 配置该cinfo，使其从jpg_buffer中读取jpg_size个字节
	// 这些数据必须是完整的JPEG数据
	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);


	// 读取JPEG文件的头，并判断其格式是否合法
	int ret = jpeg_read_header(&cinfo, true);
	if(ret != 1)
	{
		fprintf(stderr, "[%d]: jpeg_read_header failed: "
			"%s\n", __LINE__, strerror(errno));
		exit(1);
	}


	// 开始解码
	jpeg_start_decompress(&cinfo);

	// cinfo中保存了图片文件的尺寸信息
	cinfo.output_width; // 宽
	cinfo.output_height; // 高
	cinfo.output_components; // 深：每个像素点包含的字节数

	// 图片的每一行所包含的字节数
	int row_stride = cinfo.output_width * cinfo.output_components;

	// 根据图片的尺寸大小，分配一块相应的内存rgb_buffer
	// 用来存放从jpg_buffer解压出来的图像数据
	unsigned char *rgb_buffer = calloc(1, row_stride*cinfo.output_height);

	// 循环地将图片的每一行读出并解压到rgb_buffer中
	int line = 0;
	while(cinfo.output_scanline < cinfo.output_height)
	{
		unsigned char *buffer_array[1];
		buffer_array[0] = rgb_buffer +
				(cinfo.output_scanline) * row_stride;
		jpeg_read_scanlines(&cinfo, buffer_array, 1);
	}

	// 解压完了，将jpeg相关的资源释放掉
 	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(jpg_buffer);


	// 恭喜！现在rgb_buffer中就已经有图片对应的RGB数据了
	int red_offset  = vsinfo.red.offset;
	int green_offset= vsinfo.green.offset;
	int blue_offset = vsinfo.blue.offset;

	char *lcdtmp = lcdmem;
	char *rgbtmp = rgb_buffer;

	lcdtmp +=  (y*lcd_w+ x)*(lcd_bpp/8);	//计算文件换位置显示

	int w = (lcd_w-x)<cinfo.output_width ? (lcd_w-x) : cinfo.output_width;
	int h = (lcd_h-y)<cinfo.output_height? (lcd_h-y) : cinfo.output_height;

	// 将rgb数据妥善地放入lcdmem
	for(int j=0; j<h; j++)
	{
		for(int i=0; i<w; i++)
		{
			memcpy(lcdtmp + 4*i + red_offset/8,  rgbtmp + 3*i + 0, 1);
			memcpy(lcdtmp + 4*i + green_offset/8, rgbtmp + 3*i + 1, 1);
			memcpy(lcdtmp + 4*i + blue_offset/8,   rgbtmp + 3*i + 2, 1);
		}

		lcdtmp += (lcd_w*lcd_bpp/8); // lcd显存指针向下偏移一行
		rgbtmp += (row_stride); // rgb指针向下偏移一行
	}

	// 释放相应的资源
	free(rgb_buffer);
}	

void freelcd(void)
{
	if(!first)
	{
		munmap(lcdmem, screen_size);
		close(lcd);

		first = true;
	}
}
