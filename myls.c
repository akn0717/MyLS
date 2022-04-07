//https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <glob.h>
#include <libgen.h>

#define MAX_SIZE 1024

char* path_join(char* current_path, char* file_name)
{
    int s1 = strlen(current_path);
    int s2 = strlen(file_name);
    int s = s1 + s2;
    if (current_path[s1]!='/') ++s;
    char * path_file = (char*) malloc(sizeof(char) * s + 1);
    memset(path_file, 0, s);

    strcat(path_file, current_path);
    if (path_file[s1] != '/') strcat(path_file,"/");
    
    strcat(path_file, file_name);
    path_file[s] = '\0';
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

void print_file_info(char *path, char* file_name, int mode_i, int mode_l, int mode_R)
{
    struct stat statbuf;
    lstat(path, &statbuf);

    if (mode_i) printf("%ld\t", statbuf.st_ino);

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

    if (file_name==NULL)
    {
        file_name = basename(path);
    }

    printf("%s", file_name);

    if (mode_l && S_ISLNK(statbuf.st_mode))
    {
        char buffer[MAX_SIZE];
        ssize_t len;
        if ((len = readlink(path, buffer, MAX_SIZE - 1)) == -1)
        {
            printf(" ERROR: Cannot get symbolic link path");
        }
        else
        {
            buffer[len] = '\0';
            printf(" -> %s", buffer);
        }
    }
    printf("\n");
}

int isFolder(char* path)
{
    struct stat statbuf;
    lstat(path, &statbuf);
    return (S_ISDIR(statbuf.st_mode) != 0);
}

int isValid(char *path)
{
    struct stat statbuf;
    int error = lstat(path, &statbuf);
    return (error==0);
}

void get_current_info(char *path, int mode_i, int mode_l, int mode_R, int mode_H)
{
    if (isFolder(path) == 0)
    {
        print_file_info(path, NULL, mode_i, mode_l, mode_R);
        return;
    }

    if (mode_H)
    {
        printf("%s:\n", path);
    }

    struct dirent **name_list;
    int n = scandir(path, &name_list, NULL, alphasort);
    
    
    for (int i=0;i<n;++i)
    {
        if (name_list[i]->d_name[0] !=  '.')
        {
            char *path_file = path_join(path, name_list[i]->d_name);

            print_file_info(path_file, name_list[i]->d_name, mode_i, mode_l, mode_R);

            free(path_file);
        }
    }

    if (mode_R)
    {
        for (int i=0;i<n;++i)
        {
            if (name_list[i]->d_name[0] !=  '.')
            {
                
                char * path_file = path_join(path, name_list[i]->d_name);
                if (isFolder(path_file))
                {
                    printf("\n");
                    get_current_info(path_file, mode_i, mode_l, mode_R, mode_H);
                }
                free(path_file);
            }
        }
    }

    for (int i=0;i<n;++i)
    {
        free(name_list[i]);
    }
    free(name_list);
    return;
}

void append(char **paths, int *paths_size, char *path)
{
    paths[*paths_size] = path;
    ++(*paths_size);
}

int extract_wildcard(char ** paths, int *paths_size, char *path)
{
    int i=0;
    glob_t globbuf;
    char *buffer = NULL;
    if (!glob(path, 0, NULL, &globbuf)) {
        for (i=0;  i <globbuf.gl_pathc; i++) {
            buffer = strdup(globbuf.gl_pathv[i]);
            append(paths, paths_size, buffer);
        }
        globfree(&globbuf);
    } else 
    {
        append(paths, paths_size, buffer);
        return 1;
    }
    return 0;
}

int parsing(int argc, 
            char **argv, 
            int *mode_i, 
            int *mode_l, 
            int *mode_R, 
            char **paths, 
            int *paths_size)
{
    if (argc > 1)
    {
        for(int i=1; i<argc; i++)
        {
            size_t length = strlen(argv[i]);
            if (argv[i][0] == '-')
            {
                for (int j=1; j < length; j++)
                {
                    if(argv[i][j] == 'i') *mode_i = 1;
                    else if(argv[i][j] == 'l') *mode_l = 1;
                    else if(argv[i][j] == 'R') *mode_R = 1;
                    else 
                    {
                        fprintf(stderr, "ERROR: Unsupported Option\n");
                        return 1;
                    }
                }
            }
            else
            {
                if (!isValid(argv[i]))
                {
                    fprintf(stderr, "ERROR: Nonexistent files or directories\n");
                    return 1;
                }
                append(paths, paths_size, argv[i]);
            }
        }
    }
    return 0;
}

void bubble_sort(char **argv, int l,int r)
{
    for (int i=l;i<=r;++i)
        for (int j = i+1; j <= r;++j)
        {   
            if ((isFolder(argv[i]) && !isFolder(argv[j])) || (!(isFolder(argv[i]) ^ isFolder(argv[j])) && strcmp(argv[i],argv[j]) > 0))
            {
                char* temp = argv[i];
                argv[i] = argv[j];
                argv[j] = temp;
            }
        }
}

int main(int argc, char **argv)
{
    int mode_i = 0; 
    int mode_l = 0;
    int mode_R = 0;
    int mode_H = 0;
    int paths_size = 0;
    char *path_strings[MAX_SIZE];
    memset(path_strings, 0, sizeof(char*) * MAX_SIZE);
    
    if (parsing(argc, argv, &mode_i, 
            &mode_l, &mode_R, path_strings, 
            &paths_size))
            {
                return 1;
            }
            
    bubble_sort(path_strings, 0, paths_size-1);

    if (mode_R || paths_size > 1) mode_H = 1;

    if (paths_size > 0)
    {
        for (int i=0;i<paths_size;++i)
        {
            get_current_info(path_strings[i], mode_i, mode_l, mode_R, mode_H);
            if (i+1 < paths_size)
            {
                if (isFolder(path_strings[i+1]))
                {
                    printf("\n");
                }
            }
        }
    }
    else 
    {
        get_current_info(".", mode_i, mode_l, mode_R, mode_H);
    }
    return 0;
}