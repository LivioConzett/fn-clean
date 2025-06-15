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


#define MAX_FILENAME_LENGTH 512

static const char ALLOWED_CHARS[] = {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-+:"};
static const uint8_t ALLOWED_CHARS_LENGTH = 70;

// struct for replacement
typedef struct replacement_d{
    char in[4];
    char out[4];
    uint8_t len_out;
} replacement_t;


// list of replacements
replacement_t replacements[] = {
    {"a","a",1},
    {"b","b",1},
    {"c","c",1},
    {"d","d",1},
    {"e","e",1},
    {"f","f",1},
    {"g","g",1},
    {"h","h",1},
    {"i","i",1},
    {"j","j",1},
    {"k","k",1},
    {"l","l",1},
    {"m","m",1},
    {"n","n",1},
    {"o","o",1},
    {"p","p",1},
    {"q","q",1},
    {"r","r",1},
    {"s","s",1},
    {"t","t",1},
    {"u","u",1},
    {"v","v",1},
    {"w","w",1},
    {"x","x",1},
    {"y","y",1},
    {"z","z",1},
    
    {"A","A",1},
    {"B","B",1},
    {"C","C",1},
    {"D","D",1},
    {"E","E",1},
    {"F","F",1},
    {"G","G",1},
    {"H","H",1},
    {"I","I",1},
    {"J","J",1},
    {"K","K",1},
    {"L","L",1},
    {"M","M",1},
    {"N","N",1},
    {"O","O",1},
    {"P","P",1},
    {"Q","Q",1},
    {"R","R",1},
    {"S","S",1},
    {"T","T",1},
    {"U","U",1},
    {"V","V",1},
    {"W","W",1},
    {"X","X",1},
    {"Y","Y",1},
    {"Z","Z",1},

    {"1","1",1},
    {"2","2",1},
    {"3","3",1},
    {"4","4",1},
    {"5","5",1},
    {"6","6",1},
    {"7","7",1},
    {"8","8",1},
    {"9","9",1},
    {"0","0",1},

    {"_","_",1},
    {"-","-",1},
    {"+","+",1},
    {":",":",1},

    {"ä","ae",2},
    {"ö","oe",2}
};

uint16_t REPLACEMENTS_LENGTH = sizeof(replacements) / sizeof(replacement_t);



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
 * @brief returns the length of a utf-8 char
 * @param c character to get the length of
 * @return amount of bytes the char has
 */
static int8_t get_char_length(char c){
    
    // 4-byte character (11110XXX)
    if ((c & 0b11111000) == 0b11110000)
        return 4;

    // 3-byte character (1110XXXX)
    if ((c & 0b11110000) == 0b11100000)
        return 3;

    // 2-byte character (110XXXXX)
    if ((c & 0b11100000) == 0b11000000)
        return 2;

    // 1-byte ASCII character (0XXXXXXX)
    if ((c & 0b10000000) == 0b00000000)
        return 1;

    // Probably a 10XXXXXXX continuation byte
    return -1;

}


/**
 * @brief splits up the file path into directory, filename and extension. The extension is the text after the last dot.
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
    uint16_t i = 0;

    while(filepath[i] != 0){
        if(filepath[i] == '/') slash_amount++;
        if(filepath[i] == '.') dot_amount++;
        i++;
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
 * @brief append str2 to the end of str1
 * @param str1 string to appent to
 * @param srt2 string to append
 * @return amount of bytes appended.
 */
static uint16_t string_append(char str1[], char str2[]){

    // find the end of the string
    uint16_t start = 0;
    
    while(str1[start] != 0){
        start++;
    }

    // append the string
    uint16_t i = 0;

    while(str2[i] != 0){
        str1[start + i] = str2[i];
        i++;
    }

    // cap off the str2
    str1[start+i] = 0;

    return i;
}


/**
 * @brief compare two strings
 * @param str1 first string
 * @param str2 second string
 * @return true if both strings are the same, false if not.
 */
static int8_t string_compare(char str1[], char str2[]){

    int16_t i = 0;

    while(str1[i] != 0){
        
        if(str1[i] != str2[i]){
            // fprintf(stderr,"%s == %s  false\n", str1, str2);
            return 0;
        }

        i++;
    }

    // fprintf(stderr,"%s == %s  true\n", str1, str2);
    
    return 1;
}


/**
 * @brief replace the unsafe chars of a string
 * @param length length of the strings
 * @param in the filename to change the chars of
 * @param out the new string with the changed chars
 */
static void replace_chars(uint16_t length, char *in, char *out){
    
    // fprintf(stderr,"%d %s %s\n", length, in, out);

    uint16_t i_in = 0;
    uint16_t i_out = 0;
    char str[4];


    while(in[i_in] != 0){

        // get the amount of bytes in the utf-8 string
        int8_t byte_len = get_char_length(in[i_in]);
        int8_t j = 0;

        // add the utf-8 char to a string
        for(j = 0; j < byte_len; j++){
            str[j] = in[i_in+j];
        }
        str[j] = 0;

        // fprintf(stderr, "%s\n", str);

        uint8_t replaced = 0;

        for(uint16_t r = 0; r < REPLACEMENTS_LENGTH; r++){

            if(string_compare(str, replacements[r].in)){
                string_append(out, replacements[r].out);
                replaced = 1;
                break;
            }
        }

        if(!replaced){
            string_append(out,"_");
        }
        
        // advance the counter by the amount of bytes in the utf-8 char
        if(byte_len > 0) i_in += byte_len-1;
        
        i_in++;
    }

    // printf("%s\n", out);
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
        char root_dir[MAX_FILENAME_LENGTH];
        root_dir[0] = 0;
        char filename[MAX_FILENAME_LENGTH];
        filename[0] = 0;
        char extension[MAX_FILENAME_LENGTH];
        extension[0] = 0;

        split_file_path(MAX_FILENAME_LENGTH, file, root_dir, filename, extension);

        char new_filename[MAX_FILENAME_LENGTH];
        new_filename[0] = 0;

        replace_chars(MAX_FILENAME_LENGTH, filename, new_filename);

        // + 10 because it could have gotten the ./ added and
        // copy counter that could be added.
        char new_file[MAX_FILENAME_LENGTH];

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

            char copy_file[MAX_FILENAME_LENGTH];
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
