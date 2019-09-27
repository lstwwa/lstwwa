#include "common.h"
#include "bmp.h"

void display(char *bmp, int x, int y /* (x,y)是图片左上角的坐标 */)
{
	// 0，分析BMP图片的头部信息，提取长宽深等信息
	int fd = open(bmp, O_RDWR);

	struct bitmap_header head;
	struct bitmap_info   info;

	bzero(&head, sizeof(head));
	bzero(&info, sizeof(info));
	read(fd, &head, sizeof(head));
	read(fd, &info, sizeof(info));

	// 1，读取BMP图片的具体图像数据（RGB）
	int rgb_size = head.size-sizeof(head)-sizeof(info);
	char *rgb = calloc(1, rgb_size);

	int n = 0;
	int total = 0;
	// 确保将文件中的RGB数据读完
	while(rgb_size > 0)
	{
		n = read(fd, rgb+total, rgb_size);

		rgb_size -= n;
		total    += n;
	}


	int lcd;
	int lcd_w, lcd_h, lcd_bpp;
	struct fb_var_screeninfo  vsinfo;
	char *lcdmem;

	static bool first = true;
	if(first)
	{
		// 2，打开LCD设备
		lcd = open("/dev/fb0", O_RDWR);

		// 3，获取LCD设备的硬件参数（分辨率、色深……）
		bzero(&vsinfo, sizeof(vsinfo));
		ioctl(lcd, FBIOGET_VSCREENINFO, &vsinfo); // 将LCD设备参数信息放入vsinfo中

		printf("屏幕分辨率: %d × %d\n", vsinfo.xres, vsinfo.yres);
		printf("像素点内部结构:\n");
		printf("A偏移量:%d\n", vsinfo.transp.offset);
		printf("R偏移量:%d\n", vsinfo.red.offset);
		printf("G偏移量:%d\n", vsinfo.green.offset);
		printf("B偏移量:%d\n", vsinfo.blue.offset);

		// 4，映射内存
		lcd_w = vsinfo.xres;
		lcd_h = vsinfo.yres;
		lcd_bpp = vsinfo.bits_per_pixel;
		lcdmem = mmap(NULL, lcd_w * lcd_h * lcd_bpp/8,
				PROT_READ | PROT_WRITE, MAP_SHARED, lcd, 0);


		bzero(lcdmem, lcd_w*lcd_h*lcd_bpp/8);
		first = false;
	}

	int pad = (4-(info.width*3 % 4)) % 4; // pad = 0~3

	int img_line_size = info.width*3 + pad; // 图片一行的总字节数
	int lcd_line_size = lcd_w * lcd_bpp/8 ; // 显存一行的总字节数

	// 让 rgb_r 指向最后一行
	char *rgb_r = rgb + img_line_size*(info.height-1);

	// 让显存指针指向指定的显示位置（x,y）
	char *tmp = lcdmem + (y*800+x) * 4;
	
	int w = (lcd_w-x)<info.width ? (lcd_w-x) : info.width;
	int h = (lcd_h-y)<info.height? (lcd_h-y) : info.height;

	// 5，妥善地将BMP中的RGB数据搬到映射内存上
	for(int j=0; j<h; j++)
	{
		for(int i=0; i<w; i++)
		{
			memcpy(tmp+j*lcd_line_size+4*i, (rgb_r-j*img_line_size)+3*i, 3);
		}
	}

	// 6，释放相应资源
	free(rgb);
	close(fd);

	//munmap(lcdmem, lcd_w * lcd_h * lcd_bpp/8);
	//close(lcd);
}
