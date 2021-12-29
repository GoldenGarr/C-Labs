#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


/**
 *  ID3v2/file identifier      "ID3"
     ID3v2 version              $04 00
     ID3v2 flags                %abcd0000
     ID3v2 size             4 * %0xxxxxxx
 */
typedef union {
    unsigned char buf[12];

    // 10 (12) bytes overall

    struct {
        unsigned char empty_bytes[2];   // does not read correctly otherwise
        unsigned char file_identifier[3];   // bytes 3-5
        unsigned char major_version;   // byte 6
        unsigned char revision_number;   // byte 7
        unsigned char flags;    // byte 8
        unsigned int size;  // bytes 9-12   = byte length of the frames
    } blocks;
} tag_header;

/**
 * Frame ID      $xx xx xx xx  (four characters)
     Size      4 * %0xxxxxxx
     Flags         $xx xx

 */
typedef union {
    unsigned char buf[10];

    // 10 bytes overall

    struct {
        unsigned char frame_id[4];  // bytes 1-4
        unsigned int frame_size;    // bytes 5-8
        unsigned char flag1; // bytes 9-10
        unsigned char flag2;
    } blocks;
} tag_frame;


unsigned int reverseBytes(unsigned int bytes) {
    unsigned int aux = 0;
    uint8_t byte;
    int i;

    for (i = 0; i < 32; i += 8) {
        byte = (bytes >> i) & 0xff;
        aux |= byte << (32 - 8 - i);
    }
    return aux;
}


void show(char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        printf("File not found\n");
        exit(2);
    }
    printf("--Show command\n");

    tag_header HEADER;
    fread((HEADER.buf + 2), sizeof(unsigned char), 10, file);
    printf("%s", HEADER.blocks.file_identifier);    // file_id print
    printf("v%u.%u\n", HEADER.blocks.major_version, HEADER.blocks.revision_number);    // version print
    unsigned int size = reverseBytes(HEADER.blocks.size);
    while (ftell(file) < size + 10) {
        tag_frame FRAME;
        fread(FRAME.buf, sizeof(unsigned char), 10, file);
        if (FRAME.blocks.frame_id[0] == 0) {
            break;
        }
        unsigned int frame_size = reverseBytes(FRAME.blocks.frame_size);
        unsigned char *frame_data = malloc(sizeof(unsigned char) * frame_size);
        fread(frame_data, sizeof(unsigned char), frame_size, file);
        // APIC data is too bulky to print it
        if (strcmp(FRAME.blocks.frame_id, "APIC") != 0) {
            printf("%s: ", FRAME.blocks.frame_id);
            for (int i = 0; i < frame_size; ++i) {
                printf("%c", frame_data[i]);
            }
        }
        printf("\n");
        free(frame_data);
    }

    fclose(file);
}


void get(char *file_name, char *frame_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        printf("File not found\n");
        exit(2);
    }
    printf("--Get command\n");

    tag_header HEADER;
    fread((HEADER.buf + 2), sizeof(unsigned char), 10, file);
    printf("%s", HEADER.blocks.file_identifier);    // file_id print
    printf("v%u.%u\n", HEADER.blocks.major_version, HEADER.blocks.revision_number);    // version print
    unsigned int size = reverseBytes(HEADER.blocks.size);
    while (ftell(file) < size + 10) {
        tag_frame FRAME;
        fread(FRAME.buf, sizeof(unsigned char), 10, file);
        if (FRAME.blocks.frame_id[0] == 0) {
            break;
        }
        unsigned int frame_size = reverseBytes(FRAME.blocks.frame_size);
        unsigned char *frame_data = malloc(sizeof(unsigned char) * frame_size);
        fread(frame_data, sizeof(unsigned char), frame_size, file);
        // APIC data is too bulky to print it
        if (strcmp(FRAME.blocks.frame_id, frame_name) == 0) {
            printf("%s: ", FRAME.blocks.frame_id);
            for (int i = 0; i < frame_size; ++i) {
                printf("%c", frame_data[i]);
            }
            printf("\n");
            return;
        }
        free(frame_data);
    }
    fclose(file);
}


void set(char *file_name, char *prop_name, char *prop_value) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        printf("File not found\n");
        exit(2);
    }
    printf("--Set command\n");
    // fseek(file, 0, SEEK_SET);
    tag_header HEADER;

    fread((HEADER.buf + 2), sizeof(unsigned char), 10, file);
    //  printf("%s", HEADER.blocks.file_identifier);    // file_id print
    // printf("v%u.%u\n", HEADER.blocks.major_version, HEADER.blocks.revision_number);    // version print
    unsigned int old_tag_size = reverseBytes(HEADER.blocks.size);
    unsigned int new_tag_size;
    unsigned int old_frame_size = 0;
    unsigned int new_frame_size = strlen(prop_value);


    long existing_frame_index = 0;
    bool frame_exists = false;

    tag_frame FRAME;

    while (ftell(file) < old_tag_size + 10) {
        fread(FRAME.buf, sizeof(unsigned char), 10, file);
        if (FRAME.blocks.frame_id[0] == 0) {
            break;
        }
        unsigned int frame_size = reverseBytes(FRAME.blocks.frame_size);
        unsigned char *frame_data = malloc(sizeof(unsigned char) * frame_size);
        //  printf("%ld\n", ftell(file));
        fread(frame_data, sizeof(unsigned char), frame_size, file);

        if (strcmp(FRAME.blocks.frame_id, prop_name) == 0) {
            existing_frame_index = ftell(file) - frame_size;
            old_frame_size = reverseBytes(FRAME.blocks.frame_size);
            printf("Old size of frame: %u\n", reverseBytes(FRAME.blocks.frame_size));
            printf("%s found. Old value: ", prop_name);
            for (int i = 0; i < frame_size; ++i) {
                printf("%c", frame_data[i]);
            }
            printf("\n");
            frame_exists = true;
            new_tag_size = old_tag_size - old_frame_size + new_frame_size;
            printf("%u, %u\n", old_frame_size, new_frame_size);
            printf("NEW TAG SIZEEEE: %u\n", new_tag_size);
            printf("OLD TAG SIZEEEE: %u\n", old_tag_size);
            // TODO: fix frame value size and write

            break;
        }
        free(frame_data);
    }
    printf("--Set command. Prop_name = %s, prop_value = %s\n", prop_name, prop_value);
//    FILE *write_file = fopen("bruh.mp3", "w");
//    fclose(write_file);

    FILE *write_file = fopen("bruh.mp3", "ab+");
    if (frame_exists) {
        // Rewriting an existing frame

        // Modified header size
        HEADER.blocks.size = reverseBytes(new_tag_size);
        fwrite(HEADER.buf + 2, sizeof(unsigned char), 10, write_file);
        fseek(file, 0, SEEK_SET);
        // Modified header size

        char c;
        long i = 0;
        while (i < existing_frame_index - 10) {
            c = fgetc(file);
            if (i >= 10)
                fputc(c, write_file);
            ++i;
        }

        // Write new value of a frame
        {

            FRAME.blocks.frame_size = reverseBytes(new_frame_size);
            fwrite(FRAME.buf, sizeof(unsigned char), 10, write_file);

            for (int j = 0; j < new_frame_size; ++j) {
                fputc(*(prop_value + j), write_file);
            }

        }

        printf("\n");

        fseek(file, 0, SEEK_END);
        long file_end = ftell(file);
        fseek(file, existing_frame_index + old_frame_size, SEEK_SET);

        for (long k = existing_frame_index; k < file_end; ++k) {
            c = fgetc(file);
            fputc(c, write_file);
        }

        /* long file_end = ftell(file);

         fseek(file, existing_frame_index, SEEK_SET);
         printf("FILE END: %ld\n", file_end);

         printf("FTELL: %ld\n", ftell(file)); */
    }
    fclose(write_file);
    fclose(file);
}


//��

///Users/maksim/Desktop/prog/TheMandalorian.mp3
int main(int argc, char **argv) {

    char *regex = "=";
    char *token = strtok(argv[1], regex);
    token = strtok(NULL, regex);
    char *file_name = token;

    int i = 2;
    char *prop_name;
    char *prop_value;
    while (i < argc) {
        token = strtok(argv[i], regex);
        if (strcmp(token, "--show") == 0) {
            show(file_name);
        } else if (strcmp(token, "--get") == 0) {
            token = strtok(NULL, regex);

            //printf("%s\n", token);
            get(file_name, token);
        } else if (strcmp(token, "--set") == 0) {
            token = strtok(NULL, regex);
            prop_name = token;
            ++i;
            token = strtok(argv[i], regex);
            if (strcmp(token, "--value") == 0) {
                token = strtok(NULL, regex);
                prop_value = token;
            }
            set(file_name, prop_name, prop_value);
        }
        ++i;
    }

    printf("\n Task fulfilled successfully \n");

    return 0;
}
