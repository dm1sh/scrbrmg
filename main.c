#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ACPI_DEVICE "amdgpu_bl0"
#define BACKLIGHT_PATH "/sys/class/backlight/"
#define NAME_BUFF_SIZE (sizeof(BACKLIGHT_PATH) + 20 + 15)

FILE *open_file(char *path, char *mode) {
	FILE *fp = fopen(path, mode);

	if (!fp) {
		fprintf(stderr, "Can't open file %s\n", path);
		exit(EXIT_FAILURE);	
	}

	return fp;
}

int read_file(char *path) {
	FILE *fp = open_file(path, "r");

	static char buff[5];
	fgets(buff, 5, fp);

	fclose(fp);

	return atoi(buff);
}

int main (int argc, char const *const *const argv) {
	if (argc >= 1 && (strncmp(argv[argc - 1], "-h", 2) == 0 || strncmp(argv[argc - 1], "--help", 6) == 0)) {
		printf("scrbrmg: a simple cli tool for screen brightness management.\n\n"
				"Usage: scrbrmng \t\t\tShow current brightness percentage\n"
				"   or: scrbrmng <number>\t\tSet current brightness with provided percentage\n"
				"   or: scrbrmng +<number>, -<number>\tIncrease or decreace brightness percentage by provided number\n"
				"   or: scrbrmng -h, --help\t\tShow this message\n");
		exit(EXIT_SUCCESS);
	}

	char *max_brightness_path;
	
	bool custom_device = (argc == 2 && argv[1][0] != '-' && argv[1][0] != '+' && (argv[1][0] > '9' || argv[1][0] < '0')) || (argc > 2);
	if (custom_device) {
		max_brightness_path = malloc(NAME_BUFF_SIZE);
		strcpy(max_brightness_path, BACKLIGHT_PATH);
		strncat(max_brightness_path, argv[1], 20);
		strcat(max_brightness_path, "/max_brightness");
	} else {
#ifdef ACPI_DEVICE
		max_brightness_path = BACKLIGHT_PATH ACPI_DEVICE "/max_brightness";
#else
		fprintf(stderr, "scrbrmg was compiled without default ACPI_DEVICE. You must specify it as the first argument\n");
		exit(EXIT_FAILURE);
#endif
	}

	float max_brightness = read_file(max_brightness_path);

	char* brightness_path;

	if (custom_device) {
		brightness_path = max_brightness_path;
		memcpy(brightness_path + sizeof(BACKLIGHT_PATH) + strlen(argv[1]), "brightness", 11);
	} else {
#ifdef ACPI_DEVICE
		brightness_path = BACKLIGHT_PATH ACPI_DEVICE "/brightness";
#endif
	}

	bool get_brightness = argc == 1 || (custom_device && argc == 2); 
	
	int brightness = read_file(brightness_path);
		
	if (get_brightness) {
		printf("%d\n", (int)((brightness / max_brightness) * 100 + 0.5));
		exit(EXIT_SUCCESS);
	}
	
	char const *const delta_s = argv[1 + custom_device];
	
	bool signd = delta_s[0] == '-' || delta_s[0] == '+';

	int delta = atoi(delta_s + signd);

	int res;

	float percent = max_brightness / 100;

	if (signd) {
		if (delta_s[0] == '-') {
			delta *= -1;
		}

		res = brightness + delta * percent + 0.5;
	} else {
		res = delta * percent + 0.5;
	}

	FILE *brightness_fp = open_file(brightness_path, "w");

	fprintf(brightness_fp, "%d", res);

	exit(EXIT_SUCCESS);
}

