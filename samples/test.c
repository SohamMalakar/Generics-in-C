#include <stdio.h>

template <typename T>
T myMax(T x, T y)
{
    return (x > y) ? x : y;
}

template <typename T>
struct list
{
    T data;
    struct list *next;
};

int main()
{
    // Call myMax for int
    printf("%d\n", myMax<int>(3, 7));

    // call myMax for double
    printf("%lf\n", myMax<double>(3.0, 7.0));

    // call myMax for char
    printf("%c\n", myMax<char>('g', 'e'));

    // declare a list of int
    list<int> *l = NULL;

    return 0;
}
