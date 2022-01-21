#include <stdio.h>
#include <stdlib.h>

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

template <typename T>
list<T> *CreateNode(T data)
{
    list<T> *newNode = (list<T> *)malloc(sizeof(list<T>));
    
    if (newNode == NULL)
    {
        printf("Out of memory!\n");
        exit(1);
    }
    
    newNode->data = data;
    newNode->next = NULL;
    
    return newNode;
}

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

    // create a node
    l = CreateNode<int>(3);

    // create another node
    l->next = CreateNode<int>(7);

    return 0;
}
