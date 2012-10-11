/**
 * trie tree test
 * descriptioin: make statistics on every word for its frequency
 * usage: input some strings, each followed by a 'enter' character, and end with '#'
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define Max_Char_Number 256
#define Max_Word_Len      128

struct Trie_Node
{
    int count_;
    struct Trie_Node *next_[Max_Char_Number];
};

static struct Trie_Node root = {0, {NULL}};
static char *spaces = " \t\n.\"\'()";

static int insert(const char *word)
{
    int loop = 0;
    struct Trie_Node *cur, *newnode;

    if (word[0] == '\0')
        return 0;

    cur = &root;
    for (loop = 0; ; ++loop)
    {
        if (cur->next_[word[loop]] == NULL)
        {
            newnode = (struct Trie_Node*)malloc(sizeof(struct Trie_Node));
            memset(newnode, 0, sizeof(struct Trie_Node));
            cur->next_[word[loop]] = newnode;
        }
        if (word[loop] == '\0')
            break;

        cur = cur->next_[word[loop]];
    }
    cur->count_++;

    return 0;
}

void input()
{
    char *linebuf = NULL, *line = NULL, *word = NULL;
    size_t bufsize = 0;
    int ret = 0;

    printf("> please input:\n");
    while (1)
    {
        ret = getline(&linebuf, &bufsize, stdin);
        if (ret == -1)
            break;

        line = linebuf;
        word = strsep(&line, spaces);
        /* input '#' will terminate this input */
        if (strcmp(word, "#") == 0)
            break;

        if (word[0] == '\0')
            continue;

        insert(word);
    }
}
static void printword(const char *str, int n)
{
    printf("%s\t%d\n", str, n);
}

static int traverse(struct Trie_Node *rootp)
{
    static char worddump[Max_Word_Len + 1];
    static int pos = 0;
    int i;

    if (rootp == NULL)
        return 0;

    if (rootp->count_)
    {
        worddump[pos] = '\0';
        printword(worddump, rootp->count_);
    }

    for (i = 0; i < Max_Char_Number; ++i)
    {
        worddump[pos++] = i;
        traverse(rootp->next_[i]);  /* recursive call */
        pos--;
    }

    return 0;
}

void dump(struct Trie_Node* node)
{
    static int depth = 0;
    static const char prefix[] = "    ";
    int loop = 0;
    if (node == NULL)
        return;

    for (loop = 0; loop < Max_Char_Number; loop++)
    {
        if (node->next_[loop])
        {
            printf ("%.*s", (int) depth++, prefix);
            printf("next['%d'] = %p, char = %c count = %d\n",
                loop, (node->next_[loop]), (char)loop, node->next_[loop]->count_);
            dump(node->next_[loop]);  /* recursive call */
            depth--;
        }
    }
}

int main(int argc, char *argv[])
{
    input();
    printf("\n");
    printf("---frequency---\n");
    traverse(&root);
    printf("---end---\n");

    printf("---dump(tree)---\n");
    dump(&root);
    printf("---end dump---\n");

    return 0;
}
