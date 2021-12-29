#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void extract(char *archive_name) {
    FILE *archive = fopen(archive_name, "rb");
    char *files_contained_info = malloc(sizeof(char) * 1024);
    fgets(files_contained_info, 1024, archive);

    char *regex = "*";
    char *token = strtok(files_contained_info, regex);
    int ind = 0;
    char* current_file;
    FILE* output_file;


    while (token != NULL) {
        current_file = token;
        if (strcmp(token, "\n") == 0)
            break;
        token = strtok(NULL, regex);
        long long size = strtoll(token, NULL, 10);

        printf("%s -> %lld\n", current_file, size);
        output_file = fopen(current_file, "wb");
        long long i = 0;
        while (i < size) {
            fputc(fgetc(archive), output_file);
            ++i;
        }
        fclose(output_file);
        ind += 2;
        token = strtok(NULL, regex);
    }

    fclose(archive);
}


void create(char *archive_name, char **files, int file_number) {
    FILE *archive = fopen(archive_name, "wb");
    FILE *file_read;
    char *file_name;
    long long size;
    long long sizes[file_number];
    for (int i = 0; i < file_number; ++i) {
        file_name = files[i];
        file_read = fopen(file_name, "rb");
        fseek(file_read, 0, SEEK_END);
        size = ftell(file_read);
        sizes[i] = size;
        fwrite(file_name, sizeof(char), strlen(file_name), archive);
        fwrite("*", sizeof(char), 1, archive);
        char size_as_a_string[100];
        sprintf(size_as_a_string, "%lld", size);
        fwrite(size_as_a_string, sizeof(char), strlen(size_as_a_string), archive);
        fwrite("*", sizeof(char), 1, archive);
        fclose(file_read);
    }
    fwrite("\n", sizeof(char), 1, archive);
    for (int i = 0; i < file_number; ++i) {
        printf("\n%lld\n", sizes[i]);
    }
    for (int i = 0; i < file_number; ++i) {
        file_read = fopen(files[i], "rb");
        char c;
        int index = 0;
        while (index < sizes[i]) {
            c = fgetc(file_read);
            fputc(c, archive);
            ++index;
        }
        fclose(file_read);
    }
    fclose(archive);
}


void list(char *archive_name) {
    FILE *archive = fopen(archive_name, "rb");
    char *files_contained_info = malloc(sizeof(char) * 1024);
    fgets(files_contained_info, 1024, archive);

    char *regex = "*";
    char *token = strtok(files_contained_info, regex);
    int ind = 0;
    while (token != NULL) {
        if (ind % 2 == 0 && strcmp(token, "\n") != 0) {
            printf("File included: %s\n", token);
        }
        token = strtok(NULL, regex);
        ++ind;
    }
    fclose(archive);
}


int main(int argc, char *argv[]) {
    char *archive_name;
    char **files;
    if (strcmp(argv[1], "--file") == 0) {
        archive_name = argv[2];
        int i = 3;
        while (i < argc) {
            if (strcmp(argv[i], "--extract") == 0)
                extract(archive_name);

            if (strcmp(argv[i], "--list") == 0)
                list(archive_name);

            if (strcmp(argv[i], "--create") == 0) {
                int amount = argc - i - 1;
                files = malloc(sizeof(char *) * amount);
                for (int j = 0; j < amount; ++j) {
                    files[j] = malloc(sizeof(char) * 64);
                }
                int index = 0;
                ++i;
                while (i < argc) {
                    files[index] = argv[i];
                    ++index;
                    ++i;
                }
                create(archive_name, files, amount);
            }
            ++i;
        }
    }
    return 0;
}
