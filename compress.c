#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "entropy_encoder.h"
#include "utils.h"
#include "predictor.h"
#include "compress.h"

int compress(compressConfig_t *config)
{
	// Create some variables for statistic purposes.
	double compressionStartTime = 0.0;
	double compressionEndTime = 0.0;
	double predictionEndTime = 0.0;
	unsigned int compressed_bytes = 0;
	unsigned int dump_residuals = 0;

	// Initialization of some values.
	unsigned short int *residuals = NULL;

	// Perform a few checks that the necessary options have been provided.
	if (config->samples_file[0] == '\x0')
	{
		fprintf(stderr, "\nError, please indicate the file containing the input samples to be compressed\n\n");
		return -1;
	}
	if (config->out_file[0] == '\x0')
	{
		fprintf(stderr, "\nError, please indicate the file where the compressed stream will be saved\n\n");
		return -1;
	}
	if (config->input_params.y_size * config->input_params.x_size * config->input_params.z_size == 0)
	{
		fprintf(stderr, "\nError, please specify all the x, y, and z dimensions with a number > 0\n\n");
		return -1;
	}
	if (config->input_params.in_interleaving == BI && (config->input_params.in_interleaving_depth < 1 || config->input_params.in_interleaving_depth > config->input_params.z_size))
	{
		fprintf(stderr, "\nError, the input interleaving depth has to be a positive integer not bigger than the number of bands\n\n");
		return -1;
	}
	if (config->encoder_params.out_interleaving == BI && (config->encoder_params.out_interleaving_depth < 1 || config->encoder_params.out_interleaving_depth > config->input_params.z_size))
	{
		fprintf(stderr, "\nError, the output interleaving depth has to be a positive integer not bigger than the number of bands\n\n");
		return -1;
	}
	if (config->encoder_params.encoding_method == SAMPLE && (config->encoder_params.y_0 > 8 || config->encoder_params.y_0 < 1))
	{
		fprintf(stderr, "\nError, specify a value between 1 and 8 for the initial count exponent y_0\n\n");
		return -1;
	}
	if (config->encoder_params.encoding_method == SAMPLE && (config->encoder_params.y_star > 9 || config->encoder_params.y_star < 4 || config->encoder_params.y_star < (config->encoder_params.y_0 + 1)))
	{
		fprintf(stderr, "\nError, specify a value between max{4, y_0 + 1} and 9 for the rescaling counter size parameter y_star\n\n");
		return -1;
	}
	if (config->encoder_params.encoding_method == SAMPLE && (config->encoder_params.u_max > 32 || config->encoder_params.u_max < 8))
	{
		fprintf(stderr, "\nError, specify a value between 8 and 32 for the unary length limit u_max\n\n");
		return -1;
	}
	if (config->encoder_params.encoding_method == SAMPLE && config->encoder_params.out_wordsize <= 0)
	{
		fprintf(stderr, "\nError, specify a value for the word length\n\n");
		return -1;
	}
	if (config->encoder_params.encoding_method == SAMPLE && config->init_table_file[0] == '\x0' && config->encoder_params.k == (unsigned int)-1)
	{
		fprintf(stderr, "\nError, please specify one between initialization constant and the initialization table (k and k_init_file)\n\n");
		return -1;
	}
	else if (config->init_table_file[0] != '\x0' && config->encoder_params.k != (unsigned int)-1)
	{
		fprintf(stderr, "\nError, both the initialization constant and the initialization table (k and k_init_file) are specified: only one is allowed\n\n");
		return -1;
	}
	if (config->input_params.dyn_range < 2 || config->input_params.dyn_range > 16)
	{
		fprintf(stderr, "\nError, please specify the bit width of the residuals between 2 and 16 bits\n\n");
		return -1;
	}
	if (config->encoder_params.k != (unsigned int)-1 && config->encoder_params.k > config->input_params.dyn_range - 2)
	{
		fprintf(stderr, "\nError, the initialization constant k cannot be bigger than %d\n\n", config->input_params.dyn_range - 2);
		return -1;
	}
	if (config->predictor_params.pred_bands > config->input_params.z_size)
	{
		config->predictor_params.pred_bands = config->input_params.z_size - 1;
	}
	if (config->predictor_params.register_size > 64)
	{
		fprintf(stderr, "\nError, the register size cannot be bigger than 64\n\n");
		return -1;
	}
	if (config->predictor_params.weight_resolution > 19 || config->predictor_params.weight_resolution < 4)
	{
		fprintf(stderr, "\nError, the weight resolution must be in the range [4, 19]\n\n");
		return -1;
	}
	if (config->predictor_params.weight_interval > (0x1 << 11) || config->predictor_params.weight_interval < (0x1 << 4))
	{
		fprintf(stderr, "\nError, the weight update interval must be in the range [%d, %d]\n\n", (0x1 << 11), (0x1 << 4));
		return -1;
	}
	if ((0x1 << (int)log2(config->predictor_params.weight_interval)) != config->predictor_params.weight_interval)
	{
		fprintf(stderr, "\nError, the weight update interval must be a power of 2\n\n");
		return -1;
	}
	if (config->predictor_params.weight_initial > 9 || config->predictor_params.weight_initial < -6)
	{
		fprintf(stderr, "\nError, the weight initial value must be in the range [-6, 9]\n\n");
		return -1;
	}
	if (config->predictor_params.weight_final > 9 || config->predictor_params.weight_final < -6)
	{
		fprintf(stderr, "\nError, the weight final value must be in the range [-6, 9]\n\n");
		return -1;
	}
	if (config->init_weight_file[0] != '\x0' && (config->predictor_params.weight_init_resolution > (config->predictor_params.weight_resolution + 3) || config->predictor_params.weight_init_resolution < 3))
	{
		fprintf(stderr, "\nError, the weight initial resolution must be in the range [3, %d]\n\n", config->predictor_params.weight_resolution + 3);
		return -1;
	}
	if (config->predictor_params.weight_init_resolution != 0 && config->init_weight_file[0] == '\x0')
	{
		fprintf(stderr, "\nError, either both the weight initialization table and the weight initial resolution are specified or none of them\n\n");
		return -1;
	}
	if (config->encoder_params.encoding_method == SAMPLE && (config->encoder_params.block_size != 0 || config->encoder_params.ref_interval != 0))
	{
		fprintf(stderr, "\nError, when the sample adaptive encoder is used, the block size or reference interval need not be specified, as they refer to the adaptive encoder\n\n");
		return -1;
	}
	if (config->encoder_params.encoding_method == BLOCK && (config->encoder_params.ref_interval <= 0 || config->encoder_params.ref_interval > 4096))
	{
		fprintf(stderr, "\nError, the reference interval must be a positive integer not larger than 4096\n\n");
		return -1;
	}
	if (config->encoder_params.encoding_method == BLOCK && (log2(config->encoder_params.block_size) < 3 || log2(config->encoder_params.block_size) > 6))
	{
		fprintf(stderr, "\nError, either block size must be equal to 8, 16, 32, or 64\n\n");
		return -1;
	}

	// Now can allocate the accumulation constant table, either
	// with all constant values or with the specified accumulator table
	if ((config->encoder_params.k_init = (unsigned int *)malloc(config->input_params.z_size * sizeof(unsigned int))) == NULL)
	{
		fprintf(stderr, "\nError, in allocating the accumulator initialization table\n\n");
		return -1;
	}
	if (config->init_table_file[0] != '\x0')
	{
		if (parse_acc_table(config->init_table_file, config->encoder_params.k_init, 0x0F, config->input_params.z_size) != 0)
		{
			fprintf(stderr, "\nError, in parsing the accumulator initialization table\n\n");
			return -1;
		}
	}
	else
	{
		int i = 0;
		for (i = 0; i < config->input_params.z_size; i++)
		{
			config->encoder_params.k_init[i] = config->encoder_params.k;
		}
	}

	// Now allocate the weights table, if needed.
	if (config->init_weight_file[0] != '\x0')
	{
		int i = 0;
		int prediction_len = config->predictor_params.pred_bands;
		if (config->predictor_params.full != 0)
			prediction_len += 3;
		if ((config->predictor_params.weight_init_table = (int **)malloc(sizeof(int *) * config->input_params.z_size)) == NULL)
		{
			fprintf(stderr, "\nError, in allocating the weight initialization table - 1\n\n");
			return -1;
		}
		for (i = 0; i < config->input_params.z_size; i++)
		{
			if ((config->predictor_params.weight_init_table[i] = (int *)malloc(sizeof(int) * prediction_len)) == NULL)
			{
				fprintf(stderr, "\nError, in allocating the weight initialization table - 2\n\n");
				return -1;
			}
		}
		if (parse_weights_table(config->init_weight_file, config->predictor_params.weight_init_table,
								(0x1 << (config->predictor_params.weight_init_resolution - 1)) - 1, -1 * (0x1 << (config->predictor_params.weight_init_resolution - 1)),
								config->input_params.z_size * prediction_len, prediction_len) != 0)
		{
			fprintf(stderr, "\nError, in parsing the weights initialization table\n\n");
			return -1;
		}
	}

	// *********************** 
	// Here is the actual compression algorithm.

	// Create space for the residuals.
	residuals = (unsigned short int *)malloc(sizeof(unsigned short int) * config->input_params.x_size * config->input_params.y_size * config->input_params.z_size);
	if (residuals == NULL)
	{
		fprintf(stderr, "Error in allocating %lf kBytes for the residuals buffer\n\n", ((double)sizeof(unsigned short int) * config->input_params.x_size * config->input_params.y_size * config->input_params.z_size) / 1024.0);
		return -1;
	}

	// Mark the initial time of the compression algorithm.
	compressionStartTime = ((double)clock()) / CLOCKS_PER_SEC;

	// Perform the prediction part of the algorithm (computation of the residuals).
	if (predict(config->input_params, config->predictor_params, config->samples_file, residuals) != 0)
	{
		fprintf(stderr, "\nError during the computation of the residuals (i.e. prediction)\n\n");
		return -1;
	}

	// Dump the residuals into an external file if requested.
	if (dump_residuals != 0)
	{
		// Dumps the residuals as unsigned short int (16 bits each) in little endian format in
		// BIP order
		char residuals_name[100];
		FILE *residuals_file = NULL;
		int x = 0, y = 0, z = 0;
		sprintf(residuals_name, "residuals_%s.bip", config->out_file);
		if ((residuals_file = fopen(residuals_name, "w+b")) == NULL)
		{
			fprintf(stderr, "\nError in creating the file holding the residuals\n\n");
			return -1;
		}
		for (y = 0; y < config->input_params.y_size; y++)
		{
			for (x = 0; x < config->input_params.x_size; x++)
			{
				for (z = 0; z < config->input_params.z_size; z++)
				{
					fwrite(&(MATRIX_BSQ_INDEX(residuals, config->input_params, x, y, z)), 2, 1, residuals_file);
				}
			}
		}
		fclose(residuals_file);
	}

	// Close the prediction statistics.
	predictionEndTime = ((double)clock()) / CLOCKS_PER_SEC;
	compressed_bytes = encode(config->input_params, config->encoder_params, config->predictor_params, residuals, config->out_file);
	compressionEndTime = ((double)clock()) / CLOCKS_PER_SEC;

	// Deallocate all the memory used by this function.
	if (config->encoder_params.k_init != NULL)
		free(config->encoder_params.k_init);
	if (config->predictor_params.weight_init_table != NULL)
	{
		int i = 0;
		for (i = 0; i < config->input_params.z_size; i++)
		{
			if (config->predictor_params.weight_init_table[i] != NULL)
				free(config->predictor_params.weight_init_table[i]);
		}
		free(config->predictor_params.weight_init_table);
	}
	if (residuals != NULL)
		free(residuals);
	
	// Print out some statistics.
	printf("Overall Compression duration %lf (sec)\n", compressionEndTime - compressionStartTime);
	printf("Prediction duration %lf (sec)\n", predictionEndTime - compressionStartTime);
	printf("Encoding duration %lf (sec)\n", compressionEndTime - predictionEndTime);
	printf("%d bytes (%.2lf kb) in the compressed image\n", compressed_bytes, ((double)compressed_bytes) / (1024.0));
	printf("Compressed rate %lf bits/sample\n", ((double)compressed_bytes * 8) / (config->input_params.y_size * config->input_params.x_size * config->input_params.z_size));

	return 0;
}