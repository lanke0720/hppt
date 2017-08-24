#include "sql_api.h"

int main()
{
    sqlApi my_sql ("127.0.0.1", 3306);
    my_sql.connect();
    my_sql.select();
    return 0;
}
