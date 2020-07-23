#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _COMPRESS_CCSDS123_H_
#define _COMPRESS_CCSDS123_H_

/**
 * @file compress_ccsds123.h
 * @brief This file represents the interface to the CCSDS 123 compression algorithm.
 * The user of this library will have to fill in the compressConfig data structure properly
 * and call the compress function on it. The library will perform some checks that the 
 * parameters are valid and, if so, will compress the image provided.
 * 
 * TODO: if there was any problem during compression, the library will return an error message.
 * 
 * TODO: the compress function will also return a structure with some statistics like compression
 * time, bytes of the compressed image, etc.
 */

#include "predictor.h"
#include "entropy_encoder.h"

/**
 * @typedef compressConfig_t
 * @brief this is the main configuration structure that has to be filled in to call the compression algorithm.
 * @param samples_file the name of the file where the binary samples have been written to.
 * @param out_file the name of the file where the compressed image has to be written.
 * @param init_table_file optional, name of the file where the initial table is.
 * @param init_weight_file optional, file where the initial weights are specified.
 * @param input_params characteristics of the input image (size, resolution, mode).
 * @param encoder_params parameters that control the encoding stage.
 * @param predictor_params parameters that control the prediction stage of the algorithm.
 */
typedef struct compressConfig
{
	char samples_file[128];
	char out_file[128];
	char init_table_file[128];
	char init_weight_file[128];
	input_feature_t input_params;
	encoder_config_t encoder_params;
	predictor_config_t predictor_params;
} compressConfig_t;

/**
 * @brief Receives the configuration for the compression algorithm and performs compression.
 * @param config structure where all the configuration is available.
 * @retval 0 if compression went OK.
 * @retval -1 if compression ran into any problem.
 */
int compress_ccsds123(compressConfig_t *config);

#endif

#ifdef __cplusplus
}
#endif
