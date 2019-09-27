#include "common.h"
#include "bmp.h"

int main(int argc, char **argv) // ./main xxx.bmp
{
	if(argc != 2)
	{
		printf("用法: %s <BMP图片>\n", argv[0]);
		exit(0);
	}

	display(argv[1], 0, 0);

	return 0;
}
