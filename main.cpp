#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
#include <sys/stat.h>

#include "compress_ccsds123.h"
#include "decompress_ccsds123.h"

// Folder where the results of the test will be stored.
#define RESULTS_FOLDER "./test_results/"

// Define the names of the test images (where samples are stored in plain text).
#define INPUT_SMALL "./test_images/small_surface.jpg.raw"
#define INPUT_MEDIUM "./test_images/medium_astronauts.jpg.raw"
#define INPUT_LARGE "./test_images/large_footprint.jpg.raw"

// Parts of the results names.
#define ORIGINAL "original.arr"
#define COMPRESSED "compressed.arr"
#define DECOMPRESSED "decompressed.arr"

// For each of the test images, I actually copy the one band data this number of times.
#define NUM_BANDS 10

// Example definition of an image that works well with the CCSDS 123 library.
struct Image {
	size_t numBands;
	size_t numRows;
	size_t numCols;
	std::vector<std::vector<std::vector<unsigned short>>> samples;
};

/// @brief Reads a text file that contains the samples of an image (only one band or wavelength).
/// These samples are unsigned short integers, separated by spaces inside one row, with rows
/// separated by a newline character. It loads this information in a three dimensional matrix
/// where every wavelength in the NUM_BANDS possibilities receives a copy of the original image.
/// (In theory, we have different values for each wavelength, this main.cpp is just for testing purposes...).
/// @param image where we want to store the result of the loading.
/// @param filename name of the text file where the samples are stored in the format described above.
int readSamplesFromTextFile(Image &image, const std::string filename);

/// @brief Writes a multispectral image to an output binary file in BSQ format. In this format, two bytes
/// will be used to represent each unsigned short sample. First the first band will be written to the file:
/// the first row, then the second row, etc. After the first band has been written, the second one is written
/// and so on. This binary file is what will be actually ingested by the compression algorithm.
/// Read more about BSQ format at https://desktop.arcgis.com/es/arcmap/10.3/manage-data/raster-and-images/bil-bip-and-bsq-raster-files.htm
/// @param image where we have already loaded the image samples.
/// @param filename name of the file where we want to store the samples in binary format.
int writeSamplesToBinaryFile(const Image &image, const std::string filename);

/// This main will load image samples from a text file, write them into an "original" binary
/// file, perform compression on that file, perform decompression on the outputted file and
/// return with errors if any of the steps does not happen correctly.
int main(void) {

	// Store the names of the test images and create the results folder.
	int numTests = 3;
	std::string testFiles[numTests] = {INPUT_SMALL, INPUT_MEDIUM, INPUT_LARGE};
	if (mkdir(RESULTS_FOLDER, 0777) != 0 && errno != EEXIST) {
		std::cout << "ERROR: could not create the results folder" << std::endl;
		return -1;
	}
	
	// Run each of the tests.
	std::string originalFilename, compressedFilename, decompressedFilename;
	for (int i = 0; i < numTests; i++) {

		// IMAGE LOADING
		std::cout << "\nLoading image samples from file " << testFiles[i] << std::endl;
		Image image;
		if (readSamplesFromTextFile(image, testFiles[i]) != 0) {
			std::cout << "ERROR: could not read samples from " << testFiles[i] << std::endl;
			return -1;
		}
		std::cout << "Samples correctly read from " << testFiles[i] << std::endl;

		// Write the samples of the image to a binary file.
		originalFilename = RESULTS_FOLDER + std::to_string(i) + "_" + ORIGINAL;
		std::cout << "Writing image samples to binary file " << originalFilename << std::endl;
		if (writeSamplesToBinaryFile(image, originalFilename) != 0) {
			std::cout << "ERROR: could not write samples to file " << originalFilename << std::endl;
			return -1;
		}
		std::cout << "Image samples correctly written to " << originalFilename << std::endl;

		// COMPRESSION

		// Create the compression configuration struct and clean it.
		// (Remember this is a C like structure).
		compressConfig_t config;
		memset(&config, 0x00, sizeof(config));

		// Fill in values of the configuration.
		compressedFilename = RESULTS_FOLDER + std::to_string(i) + "_" + COMPRESSED;
		strcpy(config.samples_file, originalFilename.c_str());
		strcpy(config.out_file, compressedFilename.c_str());
		
		config.input_params.dyn_range = 16;
		config.input_params.x_size = image.numBands;
		config.input_params.y_size = image.numRows;
		config.input_params.z_size = image.numCols;
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
		decompressedFilename = RESULTS_FOLDER + std::to_string(i) + "_" + DECOMPRESSED;
		strcpy(decompressConfig.in_file, compressedFilename.c_str());
		strcpy(decompressConfig.out_file, decompressedFilename.c_str());
		decompressConfig.input_params.in_interleaving = BSQ;

		// Perform the decompression algorithm.
		std::cout << "\nDecompressing..." << std::endl;
		if (decompress(&decompressConfig) != 0) {
			std::cout << "ERROR: there was a problem during decompression" << std::endl;
			return -1;
		}
		std::cout << "SUCCESS: decompression went well" << std::endl;
	}

	std::cout << "\nSUCCESS: process finished succesfully" << std::endl;
	std::cout << "Check the byte sizes of the original and compressed images using" << std::endl;
	std::cout << "\twc -c < <filename>" << std::endl;
	std::cout << "Check if the original and decompressed binary files differ using diff" << std::endl;

	return 0;
}

int writeSamplesToBinaryFile(const Image &image, const std::string filename) {

	// Open the file to write to.
	std::ofstream file;
	file.open(filename);
	if (file.fail()) {
		std::cout << "ERROR: could not open file to write" << std::endl;
		return -1;
	}

	// Loop through the bands, rows and lastly columns.
	for (size_t k = 0; k < image.numBands; k++) {
		for (size_t i = 0; i < image.numRows; i++) {
			for (size_t j = 0; j < image.numCols; j++) {
				file.write(reinterpret_cast<const char *>(&image.samples[k][i][j]), sizeof(unsigned short));
			}
		}
	}

	file.close();
	return 0;
}

int readSamplesFromTextFile(Image &image, const std::string filename) {

	// Open file to read.
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

	// Check the number of rows in the image.
	if (matrix.size() != currentRow) {
		std::cout << "ERROR: the number of rows in the input image is not consistent" << std::endl;
		return -1;
	}
	if (currentRow == 0) {
		std::cout << "ERROR: please provide an image with at least one row" << std::endl;
		return -1;
	}

	// Check the number of columns in the image.
	size_t width = matrix[0].size();
	for (size_t i = 1; i < currentRow; i++) {
		if (matrix[i].size() != width) {
			std::cout << "ERROR: not all the rows in the input image have the same length" << std::endl;
			return -1;
		}
	}

	// On each band of the image, write a copy of the image extracted from the file.
	image.samples.clear();
	for (int k = 0; k < NUM_BANDS; k++) {
		image.samples.push_back(std::vector<std::vector<unsigned short>>());
		for (size_t i = 0; i < currentRow; i++) {
			image.samples[k].push_back(std::vector<unsigned short>());
			for (size_t j = 0; j < width; j++) {
				image.samples[k][i].push_back(matrix[i][j]);
			}
		}
	}

	// Write the dimensions of the image.
	image.numBands = NUM_BANDS;
	image.numRows = currentRow;
	image.numCols = width;

	return 0;
}