#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct
{
    int count;
    char character;
} entry_t;

typedef struct __arg
{
    char *src;
    int size;
    entry_t *entries;
    int entry_count;
} arg_t;

void *worker(void *thread_arg)
{
    arg_t *arg = thread_arg;
    char *src = arg->src;
    int length = arg->size;
    entry_t *entries = malloc(length * sizeof(entry_t));
    int entry_count = 0;
    int current_char = -1;
    int count = 0;

    for (int i = 0; i < length; i++)
    {
        if (src[i] == current_char)
        {
            count++;
        }
        else
        {
            if (current_char != -1)
            {
                entries[entry_count].count = count;
                entries[entry_count].character = (char)current_char;
                entry_count++;
            }
            current_char = src[i];
            count = 1;
        }
    }
    entries[entry_count].count = count;
    entries[entry_count].character = (char)current_char;
    entry_count++;

    arg->entries = entries;
    arg->entry_count = entry_count;

    return NULL;
}

int main(int argc, char *argv[])
{
    int current_char = -1;
    int count = 0;

    if (argc == 1)
    {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    for (int file_index = 1; file_index < argc; file_index++)
    {
        int fd = open(argv[file_index], O_RDONLY);

        struct stat file_info;
        fstat(fd, &file_info);
        int fsize = (int)file_info.st_size;

        if (fsize == 0)
        {
            close(fd);
            continue;
        }

        char *src = mmap(0, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);

        if (fsize > 4096)
        {
            int chunk_size = fsize / 3;
            pthread_t threads[3];
            arg_t thread_args[3];

            for (int i = 0; i < 3; i++)
            {
                thread_args[i].src = src + i * chunk_size;
                if (i == 2)
                {
                    thread_args[i].size = fsize - i * chunk_size;
                }
                else
                {
                    thread_args[i].size = chunk_size;
                }
                pthread_create(&threads[i], NULL, worker, &thread_args[i]);
            }

            for (int i = 0; i < 3; i++)
            {
                pthread_join(threads[i], NULL);
            }

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < thread_args[i].entry_count; j++)
                {
                    entry_t current_entry = thread_args[i].entries[j];
                    if (current_entry.character == current_char)
                    {
                        count += current_entry.count;
                    }
                    else
                    {
                        if (current_char != -1)
                        {
                            fwrite(&count, sizeof(int), 1, stdout);
                            fwrite(&current_char, sizeof(char), 1, stdout);
                        }
                        current_char = current_entry.character;
                        count = current_entry.count;
                    }
                }
                free(thread_args[i].entries);
            }
        }
        else
        {
            for (int i = 0; i < fsize; i++)
            {
                if (src[i] == current_char)
                {
                    count++;
                }
                else
                {
                    if (current_char != -1)
                    {
                        fwrite(&count, sizeof(int), 1, stdout);
                        fwrite(&current_char, sizeof(char), 1, stdout);
                    }
                    current_char = src[i];
                    count = 1;
                }
            }
        }

        munmap(src, fsize);
    }

    if (current_char != -1)
    {
        fwrite(&count, sizeof(int), 1, stdout);
        fwrite(&current_char, sizeof(char), 1, stdout);
    }

    return 0;
}