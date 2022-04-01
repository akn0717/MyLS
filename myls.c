//https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

char* path_join(char* current_path, char* file_name)
{
    int m = strlen(current_path) + strlen(file_name) + 1;
    char * path_file = malloc(sizeof(char) * (m+1));
    memset(path_file, 0, m);
    strcat(path_file, current_path);
    strcat(path_file,"/");
    strcat(path_file, file_name);
    path_file[m] = '\0';
    return path_file;
}

void print_permission(mode_t mode)
{
    printf("%c", S_ISDIR(mode) ? 'd' : '-');
    printf("%c", (mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (mode & S_IXUSR) ? 'x' : '-');
    printf("%c", (mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (mode & S_IXGRP) ? 'x' : '-');
    printf("%c", (mode & S_IROTH) ? 'r' : '-');
    printf("%c", (mode & S_IWOTH) ? 'w' : '-');
    printf("%c", (mode & S_IXOTH) ? 'x' : '-');
}

typedef unsigned int id;

char* get_username(id uid)
{
    struct passwd *pw = getpwuid(uid);
    char *name = NULL;
    if (pw) 
    {
        int n = strlen(pw->pw_name);
        name = (char*) malloc(sizeof(char) * (n+1));
        strcpy(name, pw->pw_name);
        name[n] = '\0';
    }
    return name;
}

char* get_groupname(id gid)
{
    struct group *grp = getgrgid(gid);
    char *name = NULL;
    if (grp) 
    {
        int n = strlen(grp->gr_name);
        name = (char*) malloc(sizeof(char) * (n+1));
        strcpy(name, grp->gr_name);
        name[n] = '\0';
    }
    return name;
}

char* time_parsing(time_t time)
{
    char *str_time = (char*) malloc(sizeof(char) * 100);
    struct tm *time_format = localtime(&time);
    strftime(str_time, 100, "%b %d %Y %H:%M", time_format);
    return str_time;
}


void get_current_info(int depth, char *path_name, int mode_i, int mode_l, int mode_R)
{
    if (depth >= 1)
    {
        printf("\n");
    }
    if (mode_R)
    {
        printf("%s:\n", path_name);
    }
    struct dirent **name_list;
    int n = scandir(path_name, &name_list, NULL, alphasort);
    struct stat statbuf;

    for (int i=0;i<n;++i)
    {
        if (name_list[i]->d_name[0] !=  '.')
        {
            if (mode_l)
            {
                char *path_file = path_join(path_name, name_list[i]->d_name);
                
                stat(path_file, &statbuf);
                if (mode_i)
                {
                    printf("%ld ", name_list[i]->d_ino);
                }
                print_permission(statbuf.st_mode);

                char *user_name = get_username(statbuf.st_uid);
                char *group_name = get_groupname(statbuf.st_gid);

                char *time = time_parsing(statbuf.st_mtim.tv_sec);
                printf("  %ld  %s  %s %5ld  %s  ", statbuf.st_nlink, user_name, group_name, statbuf.st_size, time);


                free(path_file);
                free(user_name);
                free(group_name);
                free(time);

            }
            printf("%s", name_list[i]->d_name);
            if (mode_l || i==n-1) 
            {
                printf("\n");
            }
            else if (!mode_l && i!=n-1)
            {
                printf(" ");
            }
        }
    }

    if (mode_R)
    {
        for (int i=0;i<n;++i)
        {
            if (name_list[i]->d_name[0] !=  '.')
            {
                
                char * sub_folder = path_join(path_name, name_list[i]->d_name);

                stat(sub_folder, &statbuf);
                if (S_ISDIR(statbuf.st_mode))
                {
                    get_current_info(depth + 1, sub_folder, mode_i, mode_l, mode_R);
                }
                free(sub_folder);
            }
        }
    }
    return;
}

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

    char path_name[1024] = ".";
    get_current_info(0, path_name, 1,1, 1);
    return 0;
}