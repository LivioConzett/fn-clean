/**
*
* gcc -std=c99 -Werror -Wall -o fn-clean.e main.c
*
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main.h"


static const char ALLOWED_CHARS[] = {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789äöüÄÖÜ_-"};
static const uint8_t ALLOWED_CHARS_LENGTH = 70;

/**
* @brief Print out the help menu
*/
static void print_help(){
    printf("\nfn-clean Help \n");
    printf("-----------------\n");
    printf("     -h  show this help screen\n");
    printf("     -t  test. Don't overwrite the filenames. Only print them out.\n");
    printf("     -v  verbose. Print what is going on\n");
    printf("<files>  files whose name to clean\n");
    printf("\n");
}


/**
 * @brief splits up the file path into directory, filename and extension
 * @param length the length of the filepath
 * @param filepath the file path to get the directory from
 * @param directory the directory that is found
 * @param filename filename found
 * @param extension extension found
 */
static void split_file_path(uint16_t length, char *filepath, char *directory, char *filename, char *extension){

    // go through the filepath and count how many '/' there are.

    uint8_t slash_amount = 0;
    uint8_t slash_counter = 0;
    uint8_t dot_amount = 0;
    uint8_t dot_counter = 0;
    uint16_t filename_counter = 0;
    uint16_t directory_counter = 0;
    uint16_t extension_counter = 0;

    for(uint16_t i = 0; i < length; i++){
        if(filepath[i] == '/') slash_amount++;
        if(filepath[i] == '.') dot_amount++;
    }

    // Go through the filepath. Push everything into the directory buffer
    // until the amount of slashes have been found. Then push everything into
    // the filename buffer.
    for(uint16_t i = 0; i < length; i++){

        if(filepath[i] == '.') dot_counter++;

        if(slash_counter < slash_amount){
            directory[directory_counter] = filepath[i];
            directory_counter++;
        }
        else if(dot_amount > 0 && dot_counter >= dot_amount){
            extension[extension_counter] = filepath[i];
            extension_counter++;
        }
        else{
            filename[filename_counter] = filepath[i];
            filename_counter++;
        }

        if(filepath[i] == '/') slash_counter++;

    }

    // if there was no directory, set it to the current one.
    if(directory_counter == 0){
        directory[0] = '.';
        directory[1] = '/';
        directory_counter = 2;
    }

    // end the filename and directory string 
    filename[filename_counter] = 0;
    directory[directory_counter] = 0;
    extension[extension_counter] = 0;

    // printf("%s - %s - %s\n", directory, filename, extension);

}

/**
 * @brief checks if a character is safe or not
 * @param c character to check
 * @return 1 if c is safe, else 0
 */
static uint8_t char_is_safe(char c){

    for(uint8_t i = 0; i < ALLOWED_CHARS_LENGTH; i++){
        if(ALLOWED_CHARS[i] == c) return 1;
    }

    return 0;
}


/**
 * @brief replace the unsafe chars of a string
 * @param in the filename to change the chars of
 * @param out the new string with the changed chars
 */
static void replace_chars(uint16_t length, char *in, char *out){

    for(uint16_t i = 0; i < length; i++){
        
        // If the end was reached, cap off the string and return.
        if(in[i] == 0){
            out[i] = 0;
            return;
        }

        if(char_is_safe(in[i])){
            out[i] = in[i];
        }
        else{
            out[i] = '_';
        }
    }
}


/**
* @brief Main entry point for the program
*/
int main(int argc, char *argv[]){

    if(argc < 2){
        fprintf(stderr, "\nfn-clean needs at least one argument\nUse -h for help\n\n");
        return 1;
    }

    // Only testing. Don't overwrite the filenames. Only print them out.
    uint8_t test_param = 0;
    uint8_t verbose_param = 0;
    // pointer to the string for the file chooser
    char *file_list[argc];

    uint16_t file_list_counter = 0;

    // go through the arguments
    for(uint16_t i = 1; i < argc; i++){
        
        // if the argument starts with a '-' it is a control argument
        if(argv[i][0] == '-'){

            switch(argv[i][1]){
                
                // show the help screen
                case 'h':
                    print_help();
                    return 0;
                // set the test param
                case 't':
                    test_param = 1;
                    break;
                // set the verbose param
                case 'v':
                    verbose_param = 1;
                    break;

                default:
                    fprintf(stderr,"\nDo not know argument -%c.\nTry -h for help.\n\n", argv[i][1]);
                    return 1;
            }
        }
        else{
            file_list[file_list_counter] = argv[i];
            file_list_counter++;
        }

    }

    // check if a file chooser was given
    if(file_list_counter == 0){
        fprintf(stderr, "\nNo file chooser was given.\nTry -h for help.\n\n");
        return 0;
    }

    // go through the files
    for(int i = 0; i < file_list_counter; i++){

        // printf("%s\n", file_list[i]);

        char* file = file_list[i];
        uint16_t file_length = strlen(file);


        // check if the file is a file
        struct stat path_stat;
        stat(file, &path_stat);
        
        if(!S_ISREG(path_stat.st_mode)){
            if(verbose_param) fprintf(stderr,"%s is not a file\n",file);
            continue;
        }

        // get the directory from the file chooser
        char root_dir[file_length];
        char filename[file_length];
        char extension[file_length];

        split_file_path(file_length, file, root_dir, filename, extension);

        char new_filename[file_length];

        replace_chars(file_length, filename, new_filename);

        // + 10 because it could have gotten the ./ added and
        // copy counter that could be added.
        char new_file[file_length + 10];

        strcpy(new_file, root_dir);
        strcat(new_file, new_filename);
        strcat(new_file, extension);


        // check if the file already exists
        uint8_t copy_counter = 1;
        uint8_t file_ok = 1;

        if(access(new_file, F_OK) == 0){
            if(verbose_param) fprintf(stderr,"DUPLICATE %s \n", new_file);
            file_ok = 0;
        }


        // Try to find a file that can be written by adding numbers behind the filename.
        while(!file_ok){

            char copy_file[file_length + 10];
            char counter_string[5];
            sprintf(counter_string, "%d", copy_counter);

            strcpy(copy_file, root_dir);
            strcat(copy_file, new_filename);
            strcat(copy_file, "_");
            strcat(copy_file, counter_string);
            strcat(copy_file, extension);

            if(verbose_param) printf("  copy: %d\n", copy_counter);

            if(access(copy_file, F_OK) != 0){
                file_ok = 1;
                strcpy(new_file, copy_file);
            }

            if(copy_counter >= 255){
                fprintf(stderr, "too many duplicates\n");
                return 0;
            }

            copy_counter++;
        }

        if(verbose_param || test_param) printf("%s -> %s\n", file, new_file);

        if(!test_param){
            rename(file, new_file);
        }

    }

    return 0;
}
