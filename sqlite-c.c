#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

/**
 * gcc -o sqlite-test sqlite-c.c -lsqlite3
 * */

int main( int argc, char **argv )
{
    sqlite3 *db;
    sqlite3_stmt * stmt;
    const char *zTail;

    //打开数据库
    int r = sqlite3_open("sqlite-test.db", &db);
    if (r)
    {
        printf("%s",sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    //创建Table
    sqlite3_prepare(db,
            "CREATE TABLE players (ID INTEGER PRIMARY KEY, name TEXT, age INTERER);",
            -1, &stmt, &zTail);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    //插入数据
    sqlite3_prepare(db,
            "INSERT INTO players (name, num) VALUES(?, ?);",
            -1, &stmt,&zTail);

    char str[] = "Kevin";
    int n = 23;
    sqlite3_bind_text(stmt, 1, str, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, n);
    r = sqlite3_step(stmt);
    if ( r != SQLITE_DONE)
    {
        printf("[Error]: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_reset(stmt);

    //插入第二个数据
    char str2[] = "Jack";
    int n2 = 16;
    sqlite3_bind_text(stmt, 1, str2, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, n2);
    r = sqltie3_step(stmt);
    if ( r != SQLITE_DONE)
    {
        printf("[Error]: %s\n", sqlite3_errmsg(db));
    }
    sqltie3_finalize(stmt);

    //查询所有数据
    sqlite3_prepare(db,
            "SELECT ID, name, num FROM players ORDER BY num;",
            -1, &stmt, &zTail);
    r = sqlite3_step(stmt);
    int number;
    int id;
    const unsigned char *name;
    while ( r == SQLITE_ROW )
    {
        id     = sqlite3_column_int(stmt, 0);
        name   = sqlite3_column_text(stmt, 1);
        number = sqlite3_column_int(stmt, 2);
        printf("ID: %d,  Name: %s,  Age: %d \n", id, name, number);

        r = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    //关闭数据库
    sqlite3_close(db);
    return 0;
}

