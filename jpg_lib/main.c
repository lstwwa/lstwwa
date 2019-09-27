#include "common.h"
#include "jpeglib.h"

void display(char *jpg, int x, int y);
void freelcd(void);

int main(int argc, char **argv) // ./jpg xx.jpg
{
	if(argc != 2)
	{
		printf("用法： %s <jpg图片>\n", argv[0]);
		exit(0);
	}

	display(argv[1], 100, 200);
	freelcd();
	return 0;
}
