/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Author : emptyhua@gmail.com
 * Create : 2011.10.18
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "pinyin.h"
#include "utf8vector.h"
#include "linereader.h"

void hz2py(const char *line,
        int line_length,
        int add_blank,
        int polyphone_support,
        int first_letter_only,
        int convert_double_char,
        int show_tones)
{
    wchar_t uni_char; 
    wchar_t last_uni_char = 0;
    const char *utf8;
    int utf8_length;

    utf8vector line_vector = utf8vector_create(line, line_length);

    while((uni_char = utf8vector_next_unichar_with_raw(line_vector, &utf8, &utf8_length)) != '\0')
    {
        if (pinyin_ishanzi(uni_char))
        {
            const char **pinyins = NULL;
            int print_count = 0;
            int count = pinyin_get_pinyins_by_unicode(uni_char, &pinyins);
            if (count == 0)
            {
                printf("%.*s", utf8_length, utf8);
            }
            else
            {
                char *tones = NULL;
                if (show_tones)
                    pinyin_get_tones_by_unicode(uni_char, &tones);

                if (add_blank && last_uni_char != 0 && !pinyin_ishanzi(last_uni_char)) printf(" ");
                for (int i = 0; i < count; i++)
                {
                    if (first_letter_only)
                    {
                        if (show_tones)
                        {
                            if (print_count > 0)
                                printf("|");

                            printf("%c", pinyins[i][0]);
                            print_count ++;
                        }
                        else
                        {
                            int has_print = 0;
                            char c = pinyins[i][0];
                            for (int j = 0; j < i; j ++)
                            {
                                if (pinyins[j][0] == c)
                                {
                                    has_print = 1;
                                    break;
                                }
                            }

                            if (!has_print)
                            {
                                if (print_count > 0)
                                    printf("|");
                                printf("%c", pinyins[i][0]);
                                print_count ++;
                            }
                            else
                            {
                                continue;
                            }
                        }
                    }
                    else
                    {
                        if (show_tones)
                        {
                            if (print_count > 0)
                                    printf("|");
                            printf("%s", pinyins[i]);
                            print_count ++;
                        }
                        else
                        {
                            int has_print = 0;
                            char *s = (char *)pinyins[i];
                            for (int j = 0; j < i; j ++)
                            {
                                if (strcmp(pinyins[j], s) == 0)
                                {
                                    has_print = 1;
                                    break;
                                }
                            }

                            if (!has_print)
                            {
                                if (print_count > 0)
                                    printf("|");
                                printf("%s", pinyins[i]);
                                print_count ++;
                            }
                            else
                            {
                                continue;
                            }
                        }
                    }

                    if (show_tones)
                        printf("%d", tones[i]);

                    if (!polyphone_support)
                        break;

                }

                if (add_blank) printf(" ");

                free(tones);
            }
            free(pinyins);
        }
        else
        {
            if (convert_double_char && uni_char > 65280 && uni_char < 65375)
            {
                printf("%c", uni_char - 65248);
            }
            else if (convert_double_char && uni_char == 12288)
            {
                printf("%c", 32);
            }
            else
            {
                printf("%.*s", utf8_length, utf8);
            }
        }
        last_uni_char = uni_char;
    }
    printf("\n");
    utf8vector_free(line_vector);
}

void useage()
{
    fprintf(stderr, "Read utf8 string from stdin and convert chinese charactors to pinyin.\n\
    \n\
    -B    Don't add blank between pinyin\n\
    -P    Disable polyphone support\n\
    -f    Show first letter of pinyin only\n\
    -t    Show tones\n\
    -d    Convert double byte charactor to single\n\
    -h    Print this help message\n\
    \n\
eg: cat foo.txt | hz2py -d\n\
    \n\
Author: emptyhua@gmail.com\n");
}

int main(int argc, char **argv)
{
    int add_blank= 1;
    int polyphone_support = 1;
    int first_letter_only = 0;
    int convert_double_char = 1;
    int show_tones = 0;

    while(1)
    {
        static struct option long_options[] =  
        {
            {"disable-blank", no_argument, NULL, 'B'}, 
            {"disable-polyphone", no_argument, NULL, 'P'},
            {"firstletter-only", no_argument, NULL, 'f'},
            {"convert-double-char", no_argument, NULL, 'd'},
            {"show-tones", no_argument, NULL, 't'},
            {"help", no_argument, NULL, 'h'},
            {0,0,0,0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "BPfdth", long_options, &option_index);

        if (c == -1) break;

        switch(c)
        {
            case 'B':
                add_blank = 0;
                break;
            case 'P':
                polyphone_support = 0;
                break;
            case 'f':
                first_letter_only = 1;
                break;
            case 'd':
                convert_double_char = 1;
                break;
            case 't':
                show_tones = 1;
                break;
            case 'h':
                useage();
                return 0;
                break;
            default:
                useage();
                return 0;
                break;
        }
    }

    linereader reader = linereader_create(STDIN_FILENO);
    int count;
    while ((count = linereader_readline(reader)) != -1)
    {
        const char *line = reader->line_buffer;
        hz2py(line, count, add_blank, polyphone_support, first_letter_only, convert_double_char, show_tones);
    }
    linereader_free(reader);
    return 0;
}

