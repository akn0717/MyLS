#include <dirent.h>
#include <stdio.h>
#include <string.h>

int parsing(int argc, 
            char **argv, 
            int *mode_i, 
            int *mode_l, 
            int *mode_R, 
            char **paths, 
            int *paths_size,
            int *max_paths_size)
{
    printf("Program name : %s\n", argv[0]);
    if (argc > 1)
    {
        for(int i=0; i<argc; i++)
        {
            size_t length = strlen(argv[i]);
            if (argv[i][0] == '-')
            {
                for (int j=0; j < length; j++)
                {
                    printf("%c\n", argv[i][j]);    /* Print each character of the string. */
                    if(argv[i][j] == 'i')
                    {*mode_i = 1;}
                    else if(argv[i][j] == 'l')
                    {*mode_l = 1;}
                    else if(argv[i][j] == 'R')
                    {*mode_R = 1;}
                }
            }
            else
            {
                if(paths_size+1 > max_paths_size){
                    paths = realloc(paths, 2*(*max_paths_size));
                    max_paths_size = (*max_paths_size) * 2;
                }
                paths[i] = argv[i];
                printf("string %d = %s \n", i, argv[i]);
            }
        }
    }
    printf("i = %d\n", mode_i);
    printf("l = %d\n", mode_l);
    printf("R = %d\n", mode_R);
}

int main(int argc, char **argv)
{
    int mode_i = 0; 
    int mode_l = 0;
    int mode_R = 0;
    int paths_size = 0;
    char **path_strings = malloc(sizeof(char*)*1);
    int max_paths_size = 1;
    parsing(&argc, &argv, &mode_i, 
            &mode_l, &mode_R, &path_strings, 
            &paths_size, &max_paths_size);


    printf("Program name : %s\n", argv[0]);
    if (argc > 1)
    {
        for(int i=0; i<argc; i++)
        {
            size_t length = strlen(argv[i]);
            if (argv[i][0] == '-')
            {
                for (int j=0; j < length; j++)
                {
                    printf("%c\n", argv[i][j]);    /* Print each character of the string. */
                    if(argv[i][j] == 'i')
                    {mode_i = 1;}
                    else if(argv[i][j] == 'l')
                    {mode_l = 1;}
                    else if(argv[i][j] == 'R')
                    {mode_R = 1;}
                }
            }
            else
            {
                printf("string %d = %s \n", i, argv[i]);
            }
        }
    }
    return 0;
}