#include <iostream>
#include <fstream>
#include "boost/multi_array.hpp"
#include <string.h>

#include "compress.h"

#define SIZE_X 500 /* Number of pixels across. */
#define SIZE_Y 500 /* Number of pixels along. */
#define SIZE_Z 300 /* Number of wavelengths */
#define SAMPLES_FILENAME "samples.arr" /* File where the image will be stored as samples. */
#define OUTPUT_FILE "output.arr" /* File where we want to store the compressed image. */

int writeSamplesToFile(boost::multi_array<unsigned short, 3> &image, std::string filename);

int main(void) {
	// Create an image as a random three dimensional matrix.
	boost::multi_array<unsigned short, 3> image;
	image.resize(boost::extents[SIZE_X][SIZE_Y][SIZE_Z]);
	for (int i = 0; i < SIZE_X; i++) {
		for (int j = 0; j < SIZE_Y; j++) {
			for (int k = 0; k < SIZE_Z; k++) {
				image[j][j][k] = rand() % USHRT_MAX;
			}
		}
	}

	// Put the image into a file, byte after byte.
	if (writeSamplesToFile(image, SAMPLES_FILENAME) != 0) {
		std::cout << "ERROR: there was a problem writing the samples to a file." << std::endl;
		return -1;
	}

	// Create the configuration struct and initialize some default values.
	compressConfig_t config;
	config.samples_file[0] = '\x0';
	config.out_file[0] = '\x0';
	config.init_table_file[0] = '\x0';
	config.init_weight_file[0] = '\x0';
	memset(&config.input_params, 0, sizeof(input_feature_t));
	memset(&config.encoder_params, 0, sizeof(encoder_config_t));
	memset(&config.predictor_params, 0, sizeof(predictor_config_t));
	config.encoder_params.k = (unsigned int)-1;
	config.input_params.dyn_range = 16;
	config.encoder_params.encoding_method = BLOCK;

	// Fill in the values of the configuration.
	strcpy(config.samples_file, SAMPLES_FILENAME);
	strcpy(config.out_file, OUTPUT_FILE);
	config.input_params.x_size = SIZE_X;
	config.input_params.y_size = SIZE_Y;
	config.input_params.z_size = SIZE_Z;
	config.input_params.in_interleaving = BSQ;

	config.encoder_params.out_interleaving = BSQ;
	config.encoder_params.encoding_method = BLOCK;
	config.encoder_params.block_size = 16;
	config.encoder_params.ref_interval = 256;
	config.encoder_params.out_wordsize = 7;

	config.predictor_params.full = 1;
	config.predictor_params.register_size = 32;
	config.predictor_params.weight_resolution = 14;
	config.predictor_params.weight_interval = 32;
	config.predictor_params.weight_initial = 6;
	config.predictor_params.weight_final = 6;

	// Perform the compression algorithm.
	if (compress(&config) != 0) {
		std::cout << "There was a problem performing the compression" << std::endl;
		return -1;
	}

	return 0;
}

// Writes the image to a file, byte after byte, in BSQ format.
int writeSamplesToFile(boost::multi_array<unsigned short, 3> &image, std::string filename) {
	std::ofstream file;
	file.open(filename);
	if (file.fail()) {
		std::cout << "ERROR: could not open file to write" << std::endl;
		return -1;
	}

	for (int k = 0; k < SIZE_Z; k++) {
		for (int j = 0; j < SIZE_Y; j++) {
			for (int i = 0; i < SIZE_X; i++) {
				file.write(reinterpret_cast<const char *>(&image[i][j][k]), sizeof(unsigned short));
			}
			file << std::endl;
		}
	}

	file.close();
	return 0;
}