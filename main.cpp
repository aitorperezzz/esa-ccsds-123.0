#include <iostream>
#include <fstream>
#include <string.h>
#include "boost/multi_array.hpp"

#include "compress.h"
#include "decompress.h"

#define SIZE_Z 30 /* Number of wavelengths */
#define INPUT "small_surface.jpg.raw" /* Text file where the image samples are stored, row after row, as integer numbers. */
#define ORIGINAL "original.arr" /* Binary file to store the original image. */
#define COMPRESSED "compressed.arr" /* Binary file to store the compressed image. */
#define DECOMPRESSED "decompressed.arr" /* Binary file to store the decompressed image. */

/// Reads a text file that contains the samples of an image (only one band or wavelength).
/// These samples are unsigned short integers, separated by spaces inside one row, with rows
/// separated by a newline character. It loads this information in a three dimensional matrix
/// where every wavelength in the SIZE_Z possibilities receives a copy of the original image.
/// (In theory, we have different values for each wavelength, this main.cpp is just for testing purposes...).
/// @param image boost three dimensional array where we want to store the result of the loading.
/// @param filename name of the text file where the samples are stored in the format described above.
int readSamplesFromTextFile(boost::multi_array<unsigned short, 3> &image, const std::string filename);

/// Writes a three dimensional image to an output binary file in BSQ format. In this format, two bytes
/// will be used to represent each unsigned short sample. First the first band will be written to the file:
/// the first row, then the second row, etc. After the first band has been written, write the second one
/// and so on. This binary file is what will be actually ingested by the compression algorithm.
/// Read more about BSQ format at https://desktop.arcgis.com/es/arcmap/10.3/manage-data/raster-and-images/bil-bip-and-bsq-raster-files.htm
/// @param image matrix where we have already stored the samples.
/// @param filename name of the file where we want to store the samples in binary format.
int writeSamplesToBinaryFile(boost::multi_array<unsigned short, 3> &image, const std::string filename);

/// This main will load image samples from a text file, load them into an "original" binary
/// file, perform compression on that file, perform decompression on the outputted file and
/// return with errors if any of the steps does not happen correctly.
int main(void) {

	// LOADING OF THE IMAGE
	std::cout << "Loading image samples from text file " << INPUT << std::endl;
	boost::multi_array<unsigned short, 3> image;
	if (readSamplesFromTextFile(image, INPUT) != 0) {
		std::cout << "ERROR: could not read samples from " << INPUT << std::endl;
		return -1;
	}
	std::cout << "Samples correctly read from " << INPUT << std::endl;

	// Write the samples of the image to a binary file.
	std::cout << "Writing image samples to binary file " << ORIGINAL << std::endl;
	if (writeSamplesToBinaryFile(image, ORIGINAL) != 0) {
		std::cout << "ERROR: could not write samples to file " << ORIGINAL << std::endl;
		return -1;
	}
	std::cout << "Image samples correctly written to " << ORIGINAL << std::endl;

	// Keep track of the size of the loaded image.
	auto imageShape = image.shape();

	// COMPRESSION

	// Create the compression configuration struct and clean it.
	// (Remember this is a C like structure, so careful).
	compressConfig_t config;
	memset(&config, 0x00, sizeof(config));

	// Fill in values of the configuration.
	strcpy(config.samples_file, ORIGINAL);
	strcpy(config.out_file, COMPRESSED);
	
	config.input_params.dyn_range = 16;
	config.input_params.x_size = imageShape[0];
	config.input_params.y_size = imageShape[1];
	config.input_params.z_size = imageShape[2];
	config.input_params.in_interleaving = BSQ;

	config.encoder_params.encoding_method = SAMPLE;
	config.encoder_params.k = (unsigned int)7;
	config.encoder_params.out_interleaving = BSQ;
	config.encoder_params.out_wordsize = 8;
	config.encoder_params.u_max = 18;
	config.encoder_params.y_star = 6;
	config.encoder_params.y_0 = 1;

	config.predictor_params.full = 1;
	config.predictor_params.register_size = 32;
	config.predictor_params.weight_resolution = 14;
	config.predictor_params.weight_interval = 32;
	config.predictor_params.weight_initial = 6;
	config.predictor_params.weight_final = 6;

	// Perform the actual compression.
	std::cout << "\nCompressing..." << std::endl;
	if (compress(&config) != 0) {
		std::cout << "ERROR: there was a problem during compression" << std::endl;
		return -1;
	}
	std::cout << "SUCCESS: compression went well" << std::endl;

	// DECOMPRESSION

	// Declare and initialize the decompression configuration structure.
	decompressConfig_t decompressConfig;
	memset(&decompressConfig, 0x00, sizeof(decompressConfig_t));
	decompressConfig.input_params.byte_ordering = LITTLE;

	// Fill in some values in the configuration.
	strcpy(decompressConfig.in_file, COMPRESSED);
	strcpy(decompressConfig.out_file, DECOMPRESSED);
	decompressConfig.input_params.in_interleaving = BSQ;

	// Perform the decompression algorithm.
	std::cout << "\nDecompressing..." << std::endl;
	if (decompress(&decompressConfig) != 0) {
		std::cout << "ERROR: there was a problem during decompression" << std::endl;
		return -1;
	}
	std::cout << "SUCCESS: decompression went well" << std::endl;

	std::cout << "\nSUCCESS: process finished succesfully" << std::endl;
	std::cout << "Check the byte sizes of the original and compressed images using" << std::endl;
	std::cout << "\twc -c < <filename>" << std::endl;
	std::cout << "Check if the original and decompressed binary files differ using diff" << std::endl;

	return 0;
}

int writeSamplesToBinaryFile(boost::multi_array<unsigned short, 3> &image, const std::string filename) {
	std::ofstream file;
	file.open(filename);
	if (file.fail()) {
		std::cout << "ERROR: could not open file to write" << std::endl;
		return -1;
	}

	auto shape = image.shape();
	for (size_t k = 0; k < shape[2]; k++) {
		for (size_t j = 0; j < shape[1]; j++) {
			for (size_t i = 0; i < shape[0]; i++) {
				file.write(reinterpret_cast<const char *>(&image[i][j][k]), sizeof(unsigned short));
			}
		}
	}

	file.close();
	return 0;
}

int readSamplesFromTextFile(boost::multi_array<unsigned short, 3> &image, const std::string filename) {
	std::ifstream file;
	file.open(filename);
	if (file.fail()) {
		std::cout << "ERROR: could not open file to read" << std::endl;
		return -1;
	}

	// Read the file line by line.
	size_t currentRow = 0;
	std::string currentLine;
	std::vector<std::vector<unsigned short>> matrix;
	while (std::getline(file, currentLine)) {
		// Add a new line to the matrix.
		matrix.push_back(std::vector<unsigned short>());

		// Get the input string stream.
		std::istringstream iss(currentLine);

		// Parse all the numbers in the line.
		int currentNumber;
		while (iss >> currentNumber) {
			matrix[currentRow].push_back(currentNumber);
		}

		// Advance the current row.
		currentRow++;
	}

	// Check the dimensions of the matrix.
	if (matrix.size() != currentRow) {
		std::cout << "ERROR: the number of rows in the input image is not consistent" << std::endl;
		return -1;
	}
	size_t width = matrix[0].size();
	for (size_t i = 1; i < currentRow; i++) {
		if (matrix[i].size() != width) {
			std::cout << "ERROR: not all the rows in the input image have the same length" << std::endl;
			return -1;
		}
	}

	// Resize the boost array with the known dimensions and fill in the values.
	image.resize(boost::extents[width][currentRow][SIZE_Z]);
	for (size_t j = 0; j < currentRow; j++) {
		for (size_t i = 0; i < width; i++) {
			for (int k = 0; k < SIZE_Z; k++) {
				image[i][j][k] = matrix[j][i];
			}
		}
	}

	return 0;
}