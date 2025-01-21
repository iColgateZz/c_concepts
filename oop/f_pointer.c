/* f_pointer.c */
#include <stdio.h>
#define F fflush(stdout)
#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4

void add(int *target, int a, int b)
{
    *target = a + b;
    return;
}

void sub(int *target, int a, int b)
{
    *target = a - b;
    return;
}

void div(int *target, int a, int b)
{
    *target = a / b;
    return;
}

void mul(int *target, int a, int b)
{
    *target = a * b;
    return;
}

int main(void)
{
    int x, y, op, result;
    void (*fp)(int *, int, int);

    while (1)
    {
        printf("+ - * / 0\n");
        scanf("%d", &op);

        switch (op)
        {
        case ADD:
            fp = add;
            break;
        case SUB:
            fp = sub;
            break;
        case MUL:
            fp = mul;
            break;
        case DIV:
            fp = div;
            break;
        default:
            return 0;
        }

        printf("A: "); F;
        scanf("%d", &x);

        printf("B: "); F;
        scanf("%d", &y);

        fp(&result, x, y);
        printf("Result is %d\n", result);
    }
    return 0;
}