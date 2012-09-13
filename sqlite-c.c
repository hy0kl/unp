#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#define SQL_BUF_LEN 2048
#define DB_NAME "player.db"

/**
 * gcc -o t.sqlite sqlite-c.c -lsqlite3 
 *
 * */

int main( int argc, char **argv )
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *zTail;
    char sql[SQL_BUF_LEN] = {0};

    //打开数据库
    int r = sqlite3_open(DB_NAME, &db);
    if (r)
    {
        printf("[Error]: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }
    else
    {
        printf("You have opened a sqlite3 database named %s successfully!\n\
Congratulations! Have fun ! ^-^ \n", DB_NAME);
    }

    //Create table 创建Table
    snprintf(sql, sizeof(sql),"CREATE TABLE players (ID INTEGER PRIMARY KEY, name TEXT, age INTERER);");
    r = sqlite3_exec(db, sql, NULL, NULL, &zTail);
    if (r)
    {
        printf("[Error]: %s\n", zTail);
    }
    else
    {
        printf("It NO table, create it success.\n");
    }

    // INSERT 插入数据
    char *test_name[] = {
        "Jerry",
        "Tom",
        "Tester"
    };
    int test_age[] = {
        11,
        23,
        56
    };
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        snprintf(sql, sizeof(sql), "INSERT INTO players (name, age) VALUES('%s', %d);",
            test_name[i], test_age[i]);
        r = sqlite3_exec(db, sql, NULL, NULL, &zTail);
        if (r)
        {
            printf("[Error]: %s\n", zTail);
        }
        else
        {
            printf("INSERT SQL: [%s] success.\n", sql);
        }
    }

    // query
    int rows   = 0;
    int column = 0;
    int total  = 0;
    int k = 0;
    char **result = NULL;
    snprintf(sql, sizeof(sql), "SELECT * FROM players;");
    r = sqlite3_get_table(db, sql, &result, &rows, &column, &zTail);
    if (SQLITE_OK != r)
    {
        printf("[Error]: %s\n", zTail);
        goto FINISH;
    }
    total = (rows + 1) * column;
    printf("[Query] rows: %d, column: %d, total: %d\n", rows, column, total);
    for (i = 0; i < total;)
    {
        for (k = i; k < i + column; k++)
        {
            printf("%s\t", result[k]);
        }
        printf("\n");

        i += column;
    }
    sqlite3_free_table(result);
    result = NULL;

FINISH:
    //关闭数据库
    sqlite3_close(db);
    return 0;
}

