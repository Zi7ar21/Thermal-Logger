
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <ctime>

int main(int argc, char** argv) {
	if(argc > 1) {
		fprintf(stderr, "%s: error: unrecognized option \'%s\'\n", argv[0], argv[1]);

		return EXIT_FAILURE;
	}

	// filename of the thermal_zone
	const char* thermal_zone = "/sys/class/thermal/thermal_zone74";

	char* thermal_zone_type; // thermal_zone type string

	// find the thermal zone type
	{
		// allocate enough memory for type_filename
		char* type_filename = (char*)malloc(sizeof(char)*FILENAME_MAX);

		// check if type_filename was allocated
		if(type_filename == NULL) {
			perror("Error: Failed to allocate memory for type_filename");

			return EXIT_FAILURE;
		}

		// we anticipate the type to be located at /sys/class/thermal/thermal_zoneX/type
		snprintf(type_filename, FILENAME_MAX-strlen(thermal_zone), "%s/type", thermal_zone);

		FILE* type_file = fopen(type_filename, "r");

		// check if type_file was opened
		if(type_file == NULL) {
			fprintf(stderr, "Error: Failed to open type_file (%s)", type_filename);

			perror("");

			free(type_filename);

			return EXIT_FAILURE;
		}

		free(type_filename);

		size_t thermal_zone_type_length;

		fseek(type_file, 0, SEEK_END);

		thermal_zone_type_length = (size_t)ftell(type_file);

		rewind(type_file);

		thermal_zone_type = (char*)malloc(sizeof(char)*thermal_zone_type_length);

		if(thermal_zone_type == NULL) {
			perror("Error");

			fclose(type_file);

			return EXIT_FAILURE;
		}

		fgets(thermal_zone_type, thermal_zone_type_length, type_file);

		fclose(type_file);
	}

	FILE* log_file = fopen("thermal-log.csv", "w");

	if(log_file == NULL) {
		perror("Error");

		free(thermal_zone_type);

		return EXIT_FAILURE;
	}

	setvbuf(log_file, NULL, _IOFBF, (size_t)262144u); // increase buffer size

	fprintf(log_file, "Time,%s", thermal_zone_type);

	fflush(log_file);

	free(thermal_zone_type);

	// allocate enough memory for temp_filename
	char* temp_filename = (char*)malloc(sizeof(char)*FILENAME_MAX);

	// check if type_filename was allocated
	if(temp_filename == NULL) {
		fprintf(stderr, "Error: Failed to allocate memory for type_filename");
		perror("");

		fclose(log_file);

		return EXIT_FAILURE;
	}

	// we anticipate the temp to be located at /sys/class/thermal/thermal_zoneX/temp
	snprintf(temp_filename, FILENAME_MAX-strlen(thermal_zone), "%s/temp", thermal_zone);

	// thermal_zone sysfs temperature file
	FILE* temp_file = fopen(temp_filename, "rb");

	// check if temp_file was opened
	if(temp_file == NULL) {
		fprintf(stderr, "Error: Failed to open temp_file (%s)", temp_filename);
		perror("");

		fclose(log_file);
		free(temp_filename);

		return EXIT_FAILURE;
	}

	timespec ts; // timestamp of the temperature measurement

	int temp; // temperature

	// logger loop
	for(int i = 0; i < 10; i++) {
		// open the thermal_zone sysfs temperature file
		//fflush(temp_file);
		freopen(temp_filename, "rb", temp_file);

		// it's likely the time of the temperature measurement is closer to AFTER temp_file has been opened
		clock_gettime(CLOCK_REALTIME, &ts);

		fscanf(temp_file, "%d", &temp); // read the temperature from the file

		// log the time and temperature
		fprintf(log_file, "%lld.%09lld,%d\n",(long long int)ts.tv_sec, (long long int)ts.tv_nsec,temp);

		// close the file (so the measurement can be updated)
		//fclose(temp_file);
	}

	fclose(log_file);

	free(temp_filename);

	return EXIT_SUCCESS;
}
