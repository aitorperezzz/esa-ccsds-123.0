#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _DECOMPRESS_CCSDS123_H_
#define _DECOMPRESS_CCSDS123_H_

/**
 * @file decompress_ccsds123.h
 * @brief This file represents the interface to the CCSDS 123 decompression algorithm.
 * The user of this library will have to fill in the decompressConfig data structure properly
 * and call the decompress function on it. The library will perform some checks that the 
 * parameters are valid and, if so, will decompress the compressed image provided.
 * 
 * TODO: if there was any problem during decompression, the library will return an error message.
 * 
 * TODO: the decompress function will also return a structure with some statistics.
 */

#include "utils.h"
#include "predictor.h"

/**
 * @typedef decompressConfig_t
 * @brief this is the main configuration structure that has to be filled in to perform decompression.
 * @param in_file name of the input file with the compressed image.
 * @param out_file name of the output file with the decompressed image.
 * @param dump_residuals if the user wants to dump the residuals to an external file or not.
 * @param input_params parameters of the original input image.
 * @param predictor_params parameters that were used in the predictor and that are needed now to decompress.
 */ 
typedef struct decompressConfig
{
	char in_file[128];
	char out_file[128];
	unsigned char dump_residuals;
	input_feature_t input_params;
	predictor_config_t predictor_params;
} decompressConfig_t;

/**
 * @brief Receives the configuration for the decompression algorithm and performs decompression.
 * @param config structure where all the configuration is available.
 * @retval 0 if decompression went OK.
 * @retval -1 if decompression ran into any problem.
 */
int decompress_ccsds123(decompressConfig_t *config);

#endif

#ifdef __cplusplus
}
#endif
