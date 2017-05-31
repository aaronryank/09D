#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wunused-result"

FILE *in;

struct {
   char value[100];
   int type;
} stack[100];
int jumps[100];

int stack_index, stack_size;

enum { UNDEF, INT, CHAR, STRING };

void set_default_stack_index(), push(), pop(), io(), math(), /*...*/ jump();

int main(int argc, char **argv)
{
    in = (argc > 1) ? fopen(argv[1],"r") : stdin;

    if (!in)
        in = stdin;
    if (fileno(in) == fileno(stdin))
        fprintf(stderr,"Warning: Jumps may not work properly when reading from STDIN.\n");

    int c;
    while ((c = getc(in)) != EOF)
    {
        switch (c)
        {
          case '0':
            set_default_stack_index();
            break;
          case '1':
            push();
            break;
          case '2':
            pop();
            break;
          case '3':
            io();
            break;
          case '4':
            math();
            break;
          case '9':
            jump();
            break;
          case ';':
            while ((c = getc(in)) != '\n');
            break;
          case ' ': case '\n': case 'D':   /* fallthroughs from functions */
            break;
          default:
            putchar(c);
        }
    }
} 

void set_default_stack_index(void)
{
    fscanf(in,"%dD",&stack_index);
}

void push(void)
{
    stack[stack_size].type = getc(in) - '0';

    if (stack[stack_size].type == STRING)
    {
        int i, j;
        char s[100] = {0};
        fscanf(in,"%[^D]",s);
        for (i = j = 0; s[i]; j++, i += 3)
            stack[stack_size].value[j] = (s[i]-'0')*100+(s[i+1]-'0')*10+s[i+2]-'0';
    } else
        fscanf(in,"%[^D]",stack[stack_size].value);

    stack_index = stack_size++;
}

void pop(void)
{
    int i;
    for (i = stack_index; i < stack_size; i++) {
        memcpy(stack[i].value,stack[i+1].value,100);
        stack[i].type = stack[i+1].type;
    }
    stack_index--;
    stack_size--;

    if (stack_index < stack_size)
        stack_index = stack_size;
    if (stack_index < 0)
        stack_index = 0;
}

/* super(0,0) -> read from stack, put on stack (useless)
   super(0,1) -> read from stack, print
   super(1,1) -> read from user input, print
   super(1,0) -> read from user input, put on stack */

void io(void)
{
    int from = getc(in) - '0';
    int to   = getc(in) - '0';

    char value[100];
    int type;

    if (from == 0) {
        memcpy(value,stack[stack_index].value,100);
        type = stack[stack_index].type;
    }
    else if (from == 1) {
        sprintf(value,"%.3d",getchar());
        type = CHAR;
    }

    if (to == 0) {
        memcpy(stack[stack_size].value,value,100);
        stack[stack_size].type = type;
        stack_index = stack_size++;
    }
    else if (to == 1) {
        if (type == STRING || type == INT)
            printf("%s",value);
        else if (type == CHAR) {
            int c;
            sscanf(value,"%d",&c);
            putchar(c);
        }
    }
}

#define _push(val) do{pop();pop();stack[stack_size].type=t1;sprintf(stack[stack_size].value,"%.3d",(val));stack_index=stack_size++;}while(0)

void math(void)
{
    int x;
    fscanf(in,"%dD",&x);

    register char *val1   = stack[stack_index].value;
    register char *val2   = stack[stack_index-1].value;
    register const int t1 = stack[stack_index].type;
    register const int t2 = stack[stack_index-1].type;

    int ival1, ival2;
    sscanf(val1,"%d",&ival1);
    sscanf(val2,"%d",&ival2);

    switch (x)
    {
      case 1:
        _push(ival1 + ival2);
        break;
      case 2:
        _push(ival1 - ival2);
        break;
      case 3:
        _push(ival1 * ival2);
        break;
      case 4:
        _push(ival1 / ival2);
        break;
      case 5:
        _push(ival1 % ival2);
        break;
      case 6:
        _push(ival1 & ival2);
        break;
      case 7:
        _push(ival1 | ival2);
        break;
      case 8:
        _push(ival1 ^ ival2);
        break;
      case 9:
        _push(ival1 << ival2);
        break;
      case 10:
        _push(ival1 >> ival2);
        break;
      case 12:
        _push(ival1 == ival2);
        break;
      case 13:
        _push(ival1 != ival2);
        break;
      case 14:
        _push(ival1 >= ival2);
        break;
      case 15:
        _push(ival1 <= ival2);
        break;
      case 16:
        _push(ival1 > ival2);
        break;
      case 17:
        _push(ival1 < ival2);
        break;
    }
}

/* ... */

void
jump(void)
{
    int j;
    fscanf(in,"%dD",&j);

    /* the only way to quit the program :D */
    if (j == 0)
        exit(EXIT_SUCCESS);

    if (jumps[j])
        fseek(in,jumps[j],SEEK_SET);
    else
        jumps[j] = ftell(in);
}
