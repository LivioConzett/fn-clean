/**
*
* gcc -std=c99 -Werror -Wall -o fn-clean.e main.c
*
*/

#include <stdio.h>
#include <stdint.h>
#include "main.h"

/**
* @brief
* @params
*/
void print_help(){
    printf("\nfn-clean Help \n");
    printf("-----------------\n");
    printf("-h   show this help screen\n");
    printf("-t   test. Don't overwrite the filenames. Only print them out.\n");
    printf("\n");
}


/**
* Main function
*/
int main(int argc, char *argv[]){

    if(argc < 2){
        fprintf(stderr, "\nfn-clean needs at least one argument\nUse -h for help\n\n");
        return 1;
    }

    // Only testing. Don't overwrite the filenames. Only print them out.
    uint8_t test_param = 0;


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
            
        }

    }


    return 0;
}
