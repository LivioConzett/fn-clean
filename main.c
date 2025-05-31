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
#include "main.h"


/**
* @brief Print out the help menu
*/
static void print_help(){
    printf("\nfn-clean Help \n");
    printf("-----------------\n");
    printf("     -h         show this help screen\n");
    printf("     -t         test. Don't overwrite the filenames. Only print them out.\n");
    printf("<file chooser>  file whose name to clean\n");
    printf("\n");
}


/**
 * @brief get the directory of the filepath
 * @param length the length of the filepath
 * @param filepath the file path to get the directory from
 * @param directory the directory that is found
 * @param filename filename found
 */
static void get_directory(uint16_t length, char *filepath, char* directory, char* filename){

    // go through the filepath and count how many '/' there are.

    uint8_t slash_amount = 0;
    uint8_t slash_counter = 0;
    uint16_t filename_counter = 0;
    uint16_t directory_counter = 0;

    for(uint16_t i = 0; i < length; i++){
        if(filepath[i] == '/') slash_amount++;
    }

    // Go through the filepath. Push everything into the directory buffer
    // until the amount of slashes have been found. Then push everything into
    // the filename buffer.
    for(uint16_t i = 0; i < length; i++){

        if(slash_counter < slash_amount){
            directory[directory_counter] = filepath[i];
            directory_counter ++;
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
        directory_counter ++;
    }

    // end the filename and directory string 
    filename[filename_counter] = 0;
    directory[directory_counter] = 0;
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
    // pointer to the string for the file chooser
    char *file_chooser = 0;

    // go through the arguments
    for(uint8_t i = 1; i < argc; i++){
        
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

                default:
                    fprintf(stderr,"\nDo not know argument -%c.\nTry -h for help.\n\n", argv[i][1]);
                    return 1;
            }
        }
        else{
            file_chooser = argv[i];
        }

    }

    // check if a file chooser was given
    if(file_chooser == 0){
        fprintf(stderr, "\nNo file chooser was given.\nTry -h for help.\n\n");
        return 0;
    }

    printf("file chooser: %s\n",file_chooser);

    uint16_t file_chooser_length = strlen(file_chooser);

    // get the directory from the file chooser
    char root_dir[file_chooser_length];
    char filename[file_chooser_length];

    get_directory(file_chooser_length, file_chooser, root_dir, filename);

    printf("directory: %s\n", root_dir);
    printf(" filename: %s\n", filename);

    
    printf("\n");

    // go through the folder
    DIR *directory;
    struct dirent *dir;

    directory = opendir(root_dir);

    if(directory){
        while ((dir = readdir(directory)) != NULL){
            printf("%s\n", dir->d_name);
        }
        closedir(directory);
    }
    else{
        fprintf(stderr, "\ndirectory %s not found.\n\n", root_dir);
    }


    return 0;
}
