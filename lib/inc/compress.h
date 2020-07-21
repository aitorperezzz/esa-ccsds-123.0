#ifdef __cplusplus
extern "C"
{
#endif

#ifndef COMPRESS_H
#define COMPRESS_H

#include "predictor.h"
#include "entropy_encoder.h"

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

	// Public function that handles the compression process.
	int compress(compressConfig_t *config);

#endif

#ifdef __cplusplus
}
#endif
