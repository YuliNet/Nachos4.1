#include "syscall.h"

int main()
{
    int id;
    id = Exec("mytest");
    ThreadJoin(id);
    Exit(0);
}