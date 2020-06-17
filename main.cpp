#include <iostream>

#include "compress.h"

int main(void) {
	compressConfig_t config;
	if (compress(&config) != 0) {
		std::cout << "There was a problem performing the compression" << std::endl;
		return -1;
	}
	return 0;
}