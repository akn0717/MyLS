//https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <glob.h>

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

// that's yours
void get_current_info(int depth, char *path_name, int mode_i, int mode_l, int mode_R)
{
    if (depth >= 1)
    {
        printf("\n");
        printf("%s:\n", path_name);
    }
    
    struct dirent **name_list;
    int n = scandir(path_name, &name_list, NULL, alphasort);
    struct stat statbuf;

    for (int i=0;i<n;++i)
    {
        if (name_list[i]->d_name[0] !=  '.')
        {
            if (mode_l || mode_i)
            {
                char *path_file = path_join(path_name, name_list[i]->d_name);
                
                stat(path_file, &statbuf);
                if (mode_i)
                {
                    printf("%ld\t", name_list[i]->d_ino);
                }
                if (mode_l)
                {
                    print_permission(statbuf.st_mode);

                    char *user_name = get_username(statbuf.st_uid);
                    char *group_name = get_groupname(statbuf.st_gid);

                    char *time = time_parsing(statbuf.st_mtim.tv_sec);
                    printf("\t%ld\t%s\t%s\t%5ld\t%s\t", statbuf.st_nlink, user_name, group_name, statbuf.st_size, time);

                    free(user_name);
                    free(group_name);
                    free(time);
                }
                free(path_file);
            }
            printf("%s", name_list[i]->d_name);
            if (mode_l || i==n-1) 
            {
                printf("\n");
            }
            else if (!mode_l && i!=n-1)
            {
                printf("\t");
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

void append(char **paths, int *paths_size, int *max_paths_size, char *path)
{
    if((*paths_size)+1 > (*max_paths_size))
    {
        paths = realloc(paths, 2*(*max_paths_size));
        (*max_paths_size) = (*max_paths_size) * 2;
    }
    paths[*paths_size] = path;
    ++(*paths_size);
}

void extract_wildcard(char ** paths, int *paths_size, int *max_paths_size, char *path)
{
    int i=0;
    glob_t globbuf;

    if (!glob(path, 0, NULL, &globbuf)) {
        for (i=0;  i <globbuf.gl_pathc; i++) {
            char *buffer = strdup(globbuf.gl_pathv[i]);
            append(paths, paths_size, max_paths_size, buffer);
        }
        printf("\n");
        globfree(&globbuf);
    } else 
        printf("Error: glob()\n");
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
    if (argc > 1)
    {
        for(int i=1; i<argc; i++)
        {
            size_t length = strlen(argv[i]);
            if (argv[i][0] == '-')
            {
                for (int j=0; j < length; j++)
                {
                    if(argv[i][j] == 'i') *mode_i = 1;
                    else if(argv[i][j] == 'l') *mode_l = 1;
                    else if(argv[i][j] == 'R') *mode_R = 1;
                }
            }
            else
            {
                extract_wildcard(paths, paths_size, max_paths_size, argv[i]);
            }
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int mode_i = 0; 
    int mode_l = 0;
    int mode_R = 0;
    int paths_size = 0;
    char **path_strings = malloc(sizeof(char*)*1);
    int max_paths_size = 1;
    parsing(argc, argv, &mode_i, 
            &mode_l, &mode_R, path_strings, 
            &paths_size, &max_paths_size);
    if (paths_size > 0)
    {
        for (int i=0;i<paths_size;++i)
        {
            get_current_info(0, path_strings[i], mode_i, mode_l, mode_R);
        }
    }
    else 
    {
        get_current_info(0, ".", mode_i, mode_l, mode_R);
    }
    free(path_strings);
    return 0;
}