#ifdef __cplusplus
extern "C"
{
#endif

#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include "utils.h"
#include "predictor.h"

typedef struct decompressConfig
{
	char in_file[128];
	char out_file[128];
	unsigned char dump_residuals;
	input_feature_t input_params;
	predictor_config_t predictor_params;
} decompressConfig_t;

// Public function that handles the decompression process.
int decompress(decompressConfig_t *config);

#endif

#ifdef __cplusplus
}
#endif