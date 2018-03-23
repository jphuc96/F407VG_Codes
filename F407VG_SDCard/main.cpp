#include "mbed.h"
#include "FATFileSystem.h"
#include "SDBlockDevice.h"
#include <stdio.h>
#include <errno.h>
/* mbed_retarget.h is included after errno.h so symbols are mapped to
 * consistent values for all toolchains */
#include "platform/mbed_retarget.h"

Serial pc(PA_9,PA_10,115200);

SDBlockDevice sd(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO, MBED_CONF_SD_SPI_CLK, MBED_CONF_SD_SPI_CS);
FATFileSystem fs("sd", &sd);

void return_error(int ret_val){
  if (ret_val)
    pc.printf("Failure. %d\n", ret_val);
  else
    pc.printf("done.\n");
}

void errno_error(void* ret_val){
  if (ret_val == NULL)
    pc.printf(" Failure. %d \n", errno);
  else
    pc.printf(" done.\n");
}

int main()
{
	int error = 0;
	pc.printf("Welcome to the filesystem example.\n");

	pc.printf("Opening a new file, numbers.txt.");
	FILE* fd = fopen("/sd/numbers.txt", "w+");
	errno_error(fd);

	for (int i = 0; i < 20; i++){
		pc.printf("Writing decimal numbers to a file (%d/20)\r", i);
		fprintf(fd, "%d\n", i);
	}
	pc.printf("Writing decimal numbers to a file (20/20) done.\n");

	pc.printf("Closing file.");
	fclose(fd);
	pc.printf(" done.\n");

	pc.printf("Re-opening file read-only.");
	fd = fopen("/sd/numbers.txt", "r");
	errno_error(fd);

	pc.printf("Dumping file to screen.\n");
	char buff[16] = {0};
	while (!feof(fd)){
		int size = fread(&buff[0], 1, 15, fd);
		fwrite(&buff[0], 1, size, stdout);
	}
    pc.printf("Content: %s\r\n",buff);
	pc.printf("EOF.\n");

	pc.printf("Closing file.");
	fclose(fd);
	pc.printf(" done.\n");

	pc.printf("Opening root directory.");
	DIR* dir = opendir("/sd/");
	errno_error(fd);

	struct dirent* de;
	pc.printf("Printing all filenames:\n");
	while((de = readdir(dir)) != NULL){
		pc.printf("  %s\n", &(de->d_name)[0]);
	}

	pc.printf("Closeing root directory. ");
	error = closedir(dir);
	return_error(error);
	pc.printf("Filesystem Demo complete.\n");

	while (true) {}
}

