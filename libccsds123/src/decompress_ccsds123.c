#include <stdlib.h>
#include <time.h>

#include "decompress_ccsds123.h"
#include "utils.h"
#include "unpredict.h"
#include "decoder.h"

// Implementation of public functions.

int decompress(decompressConfig_t *config)
{
	// Initialize some variables.
	double decodingStartTime = 0.0;
	double decodingEndTime = 0.0;
	double unpredictionEndTime = 0.0;
	unsigned short int *residuals = NULL;

	// Perform a few checks that the necessary options have been provided.
	if (config->in_file[0] == '\x0')
	{
		fprintf(stderr, "\nError, please indicate the file containing the input compressed file\n\n");
		return -1;
	}
	if (config->out_file[0] == '\x0')
	{
		fprintf(stderr, "\nError, please indicate the file where the decompressed image will be saved\n\n");
		return -1;
	}

	// Start the decoding time statistics.
	decodingStartTime = ((double)clock()) / CLOCKS_PER_SEC;

	// Perform decoding.
	if (decode(&config->input_params, &config->predictor_params, &residuals, config->in_file))
	{
		fprintf(stderr, "Error during the decoding stage\n");
		if (residuals != NULL)
			free(residuals);
		return -1;
	}

	// Dump the residuals if requested.
	if (config->dump_residuals != 0)
	{
		// Dumps the residuals as unsigned short int (16 bits each) in little endian format in
		// BIP order
		char residuals_name[200];
		FILE *residuals_file = NULL;
		unsigned int x = 0, y = 0, z = 0;
		sprintf(residuals_name, "residuals_%s.bip", config->out_file);
		if ((residuals_file = fopen(residuals_name, "w+b")) == NULL)
		{
			fprintf(stderr, "\nError in creating the file holding the residuals\n\n");
			if (residuals != NULL)
				free(residuals);
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

	// Close the decoding statistics.
	decodingEndTime = ((double)clock()) / CLOCKS_PER_SEC;

	// Go through the unpredict routine.
	if (unpredict(config->input_params, config->predictor_params, residuals, config->out_file))
	{
		fprintf(stderr, "Error during the un-prediction stage\n");
		if (residuals != NULL)
			free(residuals);
		return -1;
	}

	// Close the unpredict statistics.
	unpredictionEndTime = ((double)clock()) / CLOCKS_PER_SEC;

	// Free up memory if needed.
	if (residuals != NULL)
		free(residuals);

	// Finally print some stats.
	printf("Overall Decompression duration %lf (sec)\n", unpredictionEndTime - decodingStartTime);
	printf("Decoding duration %lf (sec)\n", decodingEndTime - decodingStartTime);
	printf("Unprediction duration %lf (sec)\n", unpredictionEndTime - decodingEndTime);

	return 0;
}
