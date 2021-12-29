#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>


void
execute(char *input_file_name, char *output_dir_name, unsigned int max_iteration, unsigned int generation_frequency) {


    int iterator = 1;
    int file_counter = 1;
    int **output_image;
    FILE *file;
    FILE *output_file;
    char file_name_buf[128];
    char path[128];
    strcpy(path, output_dir_name);
    sprintf(file_name_buf, "%d", file_counter);
    strcat(strcat(path, "/"), strcat(file_name_buf, ".bmp"));
    printf("%s\n", path);

    file = fopen(input_file_name, "rb");
    unsigned char *header = malloc(sizeof(unsigned char) * 54);
    fread(header, sizeof(unsigned char), 54, file);
    int width_in_pixels = abs(*(int *) &header[18]);
    int height_in_pixels = abs(*(int *) &header[22]);
    unsigned short bits_per_pixel = *(unsigned short *) &header[28];


    printf("Width: %d, height_in_pixels: %d\n", width_in_pixels, height_in_pixels);

    // for writing
    int row_size = ((int) (((float) bits_per_pixel * (float) width_in_pixels + 31) / 32.0)) * 4;
    // row size in bytes
    printf("ROW SIZE: %d\n", row_size);
    printf("BITS: %d\n", bits_per_pixel / 8);


    int **image = malloc(height_in_pixels * sizeof(int *));
    for (int i = 0; i < height_in_pixels; ++i) {
        image[i] = malloc(sizeof(int) * width_in_pixels);
    }
    unsigned char *pixel_bytes;


    for (int i = 0; i < height_in_pixels; ++i) {
        int j = 0;
        while (j < row_size) {
            pixel_bytes = malloc(sizeof(unsigned char) * bits_per_pixel / 8);
            // works only with pixel sizes % 8
            fread(pixel_bytes, sizeof(unsigned char), bits_per_pixel / 8, file);
            if ((pixel_bytes[0] & pixel_bytes[1] & pixel_bytes[2]) != 255 &&
                (j < width_in_pixels * bits_per_pixel / 8)) {
                image[i][j / (bits_per_pixel / 8)] = 1;
            } else {
                image[i][j / (bits_per_pixel / 8)] = 0;
            }
            free(pixel_bytes);
            pixel_bytes = NULL;
            j += bits_per_pixel / 8;
        }
    }

    output_image = malloc(height_in_pixels * sizeof(int *));
    for (int i = 0; i < height_in_pixels; ++i) {
        output_image[i] = malloc(sizeof(int) * width_in_pixels);
    }

    output_file = fopen(path, "wb");
    fwrite(header, sizeof(unsigned char), 54, output_file);

    for (int i = 0; i < height_in_pixels; ++i) {
        for (int j = 0; j < width_in_pixels; ++j) {
            if (image[i][j] != 0) {
                for (int k = 0; k < bits_per_pixel / 8; ++k) {
                    fputc(0, output_file);
                }
            } else {
                for (int k = 0; k < bits_per_pixel / 8; ++k) {
                    fputc(255, output_file);
                }
            }
            // fill padding
        }
        int padding_ind = width_in_pixels * bits_per_pixel / 8;
        while (padding_ind < row_size) {
            for (int k = 0; k < bits_per_pixel / 8; ++k) {
                fputc(0, output_file);
            }
            padding_ind++;
        }
    }

    iterator++;

    while (iterator <= max_iteration) {
        for (int i = 0; i < height_in_pixels; ++i) {
            for (int j = 0; j < width_in_pixels; ++j) {
                output_image[i][j] = 0;
            }
        }

        bool j_inf, j_sup, i_inf, i_sup;
        int neighbours = 0;
        for (int i = 0; i < height_in_pixels; ++i) {
            i_inf = (i - 1) >= 0;
            i_sup = (i + 1) < height_in_pixels;
            for (int j = 0; j < width_in_pixels; ++j) {
                j_inf = (j - 1) >= 0;
                j_sup = (j + 1) < width_in_pixels;

                // I want to have <2 or >3 neighbours
                if (i_inf && j_inf)
                    neighbours += image[i - 1][j - 1];
                if (i_inf)
                    neighbours += image[i - 1][j];
                if (i_inf && j_sup)
                    neighbours += image[i - 1][j + 1];
                if (j_inf)
                    neighbours += image[i][j - 1];
                if (j_sup)
                    neighbours += image[i][j + 1];
                if (i_sup && j_inf)
                    neighbours += image[i + 1][j - 1];
                if (i_sup)
                    neighbours += image[i + 1][j];
                if (i_sup && j_sup)
                    neighbours += image[i + 1][j + 1];

                if (image[i][j] == 1)
                    output_image[i][j] = 1;
                if (neighbours < 2 || neighbours > 3) {
                    output_image[i][j] = 0;
                    // death by over- or underpopulation
                }
                if (image[i][j] == 0 && neighbours == 3)
                    output_image[i][j] = 1;
                // birth
                neighbours = 0;
            }
        }

        if (iterator % generation_frequency == 0) {
            file_counter++;
            strcpy(path, output_dir_name);
            sprintf(file_name_buf, "%d", file_counter);
            strcat(strcat(path, "/"), strcat(file_name_buf, ".bmp"));

            output_file = fopen(path, "wb");
            fwrite(header, sizeof(unsigned char), 54, output_file);

            for (int i = 0; i < height_in_pixels; ++i) {
                for (int j = 0; j < width_in_pixels; ++j) {
                    if (output_image[i][j] != 0) {
                        for (int k = 0; k < bits_per_pixel / 8; ++k) {
                            fputc(0, output_file);
                        }
                    } else {
                        for (int k = 0; k < bits_per_pixel / 8; ++k) {
                            fputc(255, output_file);
                        }
                    }
                    // fill padding
                }
                int padding_ind = width_in_pixels * bits_per_pixel / 8;
                while (padding_ind < row_size) {
                    for (int k = 0; k < bits_per_pixel / 8; ++k) {
                        fputc(0, output_file);
                    }
                    padding_ind++;
                }
            }
        }

        // cleaning up
        fclose(file);
        fclose(output_file);
        //cleaning up
        //image = output_image;
        for (int i = 0; i < height_in_pixels; ++i) {
            for (int j = 0; j < width_in_pixels; ++j) {
                image[i][j] = output_image[i][j];
            }
        }
        iterator++;
    }
    free(header);
    header = NULL;

}


int main(int argc, char **argv) {

    int i = 1;
    // Game of Life params
    char *input_file_name = "input.bmp";
    char *output_dir_name = "output_directory";
    unsigned int max_iteration = 2048;
    unsigned int generation_frequency;

    while (i < argc) {
        if (strcmp(argv[i], "--input") == 0) {
            input_file_name = argv[i + 1];
        } else if (strcmp(argv[i], "--output") == 0) {
            output_dir_name = argv[i + 1];
        } else if (strcmp(argv[i], "--max_iter") == 0) {
            max_iteration = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "--dump_freq") == 0) {
            generation_frequency = atoi(argv[i + 1]);
        }
        i += 2;
    }

    printf("Input file: %s\n", input_file_name);
    printf("Output directory: %s\n", output_dir_name);
    printf("Maximal iteration: %u\n", max_iteration);
    printf("Generation frequency print: %u\n", generation_frequency);


    execute(input_file_name, output_dir_name, max_iteration, generation_frequency);
    return 0;
}
