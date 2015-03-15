#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#include "HiloCompressorEmulator.h"
#include "MsaaCompressorEmulator.h"

using namespace gpu3d;

int fd;
bool eof = false;

int next() {
	char b[4];
	int s = read(fd, b, 4);
	eof = s <= 0;
	int d = b[3] << 24
			| b[2] << 16
			| b[1] << 8
			| b[0];
	return d;
}

void readline(u32bit* data, int size) {
	char b[size*4];
	int s = read(fd, b, size*4);
	for (int i = 0; i < size; i++) {
		int p = i * 4;
		int d = b[p+3] << 24 | b[p+2] << 16
					| b[p+1] << 8 | b[p+0];
		data[i] = d;
	}
}

class cl {
	int m1, m2;
};

int main() {
	u32bit data1[64];
	u32bit data2[64];
	u32bit cdata1[64];
	u32bit cdata2[64];
	
	//HiloCompressorEmulator compr(3, 0xffffffff);
	MsaaCompressorEmulator compr(64);

	fd = open("cdata.bin", O_RDONLY);
	if (fd < 0) {
		printf("\nERROR: opening data file!!!\n");
		exit(-1);
	}

	int lines = 0;
	
	while (!eof) {
		int size = next();
		readline(data1, size);
		lines++;
		
		if (lines > 0 && size > 0)
		{
			printf(">");
			/*for (int i = 0; i < size; i++)
				printf("%08x ", data1[i]);
			printf("(%i)", size);*/
			
			memset(cdata1, 0, size*4);
			CompressorInfo info = compr.compress(data1, cdata1, size);
			
			printf(" level=%i  csize=%i\n", info.level, info.size);
			
			/*printf("*");
			for (int i = 0; i < info.size / 4; i++)
				printf("%08x ", cdata[i]);
			printf("\n");*/
			
			if (info.success) {
				memset(cdata2, 0, size*4);
				memcpy(cdata2, cdata1, info.size);
				
				//printf("+");
				memset(data2, 0, size*4);
				CompressorInfo info2 = compr.uncompress(cdata2, data2, size, info.level);
				
				if (info2.level != info.level) {
					printf("\nERROR: different levels!!!\n");
					exit(-1);
				}
				for (int i = 0; i < size; i++) {
					//printf("%08x ", data2[i]);
					if (data1[i] != data2[i]) {
						printf("\nERROR: data differences!!!\n");
						memset(cdata1, 0, size*4);
						CompressorInfo info = compr.compress(data1, cdata1, size);
						
						memset(cdata2, 0, size*4);
						memcpy(cdata2, cdata1, info.size);
										
						memset(data2, 0, size*4);
						compr.uncompress(cdata2, data2, size, info.level);
						exit(-1);
					}
				}
				//printf("\n\n");
			}
			else {
				//printf("-");
			}
		}
	}
	
	close(fd);
	return 0;
}
