/*
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * daoyutil.c
 * Performs conversions:
 *     plaintext -> daoyu source hex
 *     plaintext -> daoyu needed to build
 *     daoyu src -> daoyu needed to build
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char DEBUG = 0;

#define MEM_ERROR 21
#define UNEXPECTED_ERROR 22
#define FILE_NOT_FOUND 23

#define TXT_SRC 0
#define SRC_DAO 1

#define ERRVALUE -2
#define RETAIN -1

#define IO 0x5
#define OI 0x6

#define CONTAINS 0xF
#define debug if (DEBUG)

typedef struct BinNodeStx
{
    struct BinNodeStx*      t_l; /* Tree  Left  */
    struct BinNodeStx*      t_r; /* Tree  right */
    struct BinNodeStx*      c_l; /* Chain left  */
    struct BinNodeStx*      c_r; /* Chain right */
    char                    value;
} BinNode;

typedef struct BinNodeQueueStx
{
    BinNode*    l; /* First */
    BinNode*    r; /* Last  */
} BinNodeQueue;

BinNodeQueue* q_new();
BinNode*      q_pop(BinNodeQueue*);
void          q_push(BinNodeQueue*, BinNode*);

BinNode*      n_new();
BinNode*      n_char(unsigned char);
BinNode*      n_chain(BinNode*, BinNode*);
void          n_print(FILE*, BinNode*);

BinNode*      qn_treeify(BinNodeQueue*);
void          qn_dao(BinNodeQueue*, char);

char          getNybble(char);
char*         val_str(BinNode*);

int main(int argc, char** argv)
{
    char* const symbols = ".!/)%#>=(<:S[*$;";
    FILE* inputStream = NULL;
    FILE* outputStream = stdout;
    unsigned char character = 0;
    int functionality = TXT_SRC;
    int tempc = argc;
    int ch = 0;
    int fileSize = 0;

    BinNodeQueue* queue = NULL;

    if (argc < 2)
        return 0;

    /* Confirm file */
    if ((inputStream = fopen(argv[1], "r")) == NULL)
        return FILE_NOT_FOUND;

    /* Scan for options to change "functionality." */
    while (tempc-- > 2)
    {
        if (~strcmp(argv[tempc], "debug"))
            DEBUG = 1;

        /* Is option */
        if (argv[tempc][0] == '-')
        {

            switch(argv[tempc][1])
            {
            case 'x':
                DEBUG = 1; 
                break;
            /* Get daoyu needed to build daoyu program that daoyu source indicates */
            case 'd':
                functionality = SRC_DAO;
                break;
            /* Have an output file. */
            case 'o':
                if ((outputStream = fopen(argv[tempc + 1], "w+")) == NULL)
                    return FILE_NOT_FOUND;
                break;
            }
        }
    }

    /* Scan from file */
    while ((ch = fgetc(inputStream)) != EOF)
    {
        /* Just do printing if it's ok */
        if (functionality == TXT_SRC)
        {
            character = (unsigned char) ch;
            fputc(symbols[(character & 0xF0) >> 4], outputStream);
            fputc(symbols[ character & 0x0F      ], outputStream);
        }
        fileSize++;
    }

    fclose(inputStream);
    
    if (!functionality)
        return 0;

    /* Binary Tree Building and Traversal */
    
    /* Initialize the queue. */
    if ((queue = q_new()) == NULL)
        return MEM_ERROR;

    /* Open the stream. */
    if ((inputStream = fopen(argv[1], "r")) == NULL)
        return UNEXPECTED_ERROR;

    /* Now it's time to write in each... */
    while ((ch = fgetc(inputStream)) != EOF)
        if (functionality == SRC_DAO && (ch != '\n') && (ch != '\r') && (ch != ' '))
            qn_dao(queue, ch);

    fclose(inputStream);

    debug putchar('\n');

    n_print(outputStream, qn_treeify(queue));
    
    return 0;
}

BinNodeQueue* q_new()
{
    BinNodeQueue* queue = NULL;
    if ((queue = malloc(sizeof(BinNodeQueue))) == NULL)
        return NULL;
    (queue -> l) = NULL;
    (queue -> r) = NULL;
    return queue;
}

void q_push(BinNodeQueue* qptr, BinNode* node)
{
    if (qptr -> r != NULL)              /* If there exists a right node                          */
    {
        ((qptr -> r) -> c_r) = node;    /* Chain the last one in queue to the pushed node 8      */
        (node -> c_l) = (qptr -> r);    /* Link this node's left to that node                    */
    }
    else                                        
        (qptr -> l) = node;             /* If it is null, that means we're adding the first one. */
    (qptr -> r) = node;                 /* Set the last one in queue to the pushed node          */
    debug printf("PUSH      %s\n", val_str(node));
}

BinNode* q_pop(BinNodeQueue* qptr)
{
    BinNode* popped = (qptr -> l);      /* Get the first one in queue                            */
    
    if (qptr -> l == qptr -> r)         /* Single value condition                                */
    {
        (popped -> c_l) = NULL;
        (popped -> c_r) = NULL;
        (qptr -> l) = NULL;
        (qptr -> r) = NULL;
        debug printf("EMPT %s\n", val_str(popped));
        return popped;
    }
    (qptr -> l) = (popped -> c_r);      /* Set the next one to the first in queue                */
    ((qptr -> l) -> c_l) = NULL;        /* Unlink popped element from new-first                  */
    (popped -> c_r) = NULL;
    debug printf("POP  %s\n", val_str(popped));
    return popped;
}

BinNode* qn_treeify(BinNodeQueue* queue)
{
    while(queue -> l != queue -> r)
        q_push(queue, n_chain(q_pop(queue), q_pop(queue)));

    debug putchar('\n');

    return (queue -> l);
}

/* 
 * Push BinNodes for a char to a queue.
 * Appropriate chars for symbols go for binary of those.
 * Other non-whitespace chars are "DO NOT ALTER."
 * try to go with convention of "x"
 */
void qn_dao(BinNodeQueue* queue, char symbol)
{
    unsigned char bitselect = 0;
    char nybble = getNybble(symbol);

    if (nybble == -4)
    {
        while (nybble++)
            q_push(queue, n_char(RETAIN));
        return;
    }

    for (bitselect = 1 << 3; bitselect; bitselect >>= 1)
        q_push(queue, n_char(((bitselect & nybble) != 0)));

}

BinNode* n_new()
{
    BinNode* node = NULL;
    if ((node = malloc(sizeof(BinNode))) == NULL)
        return NULL;
    (node -> t_l) = NULL;
    (node -> t_r) = NULL;
    (node -> c_l) = NULL;
    (node -> c_r) = NULL;
    (node -> value) = 0;
    return node;
}

BinNode* n_char(unsigned char value)
{
    BinNode* node = n_new();

    /* Confirm allocation */
    if (node == NULL)
        return NULL;

    (node -> value) = value;
    return node;
}

BinNode* n_chain(BinNode* right, BinNode* left)
{
    BinNode* node = n_new();

    /* Confirm allocation */
    if (node == NULL)
        return NULL;

    /* x / 2 returns zero for unsigned values -1, 0 or 1 only */

    /* Both values are binary or RETAIN */
    if (left -> value /2 == 0 && right -> value /2 == 0)
    {
        (node -> value) = (left -> value >= 0 && right -> value >= 0) ?
                                (left  -> value == 0) ?                                        /* BOTH BINARY     */
                                    (right -> value == 0) ? 0  : OI :
                                    (right -> value == 0) ? IO : 1  :
                                (left -> value == RETAIN && right -> value == RETAIN) ?    /* RETAIN EXISTS */
                                    RETAIN : ERRVALUE;
        free(left);
        free(right);
        return node;
    }
    /* Otherwise this node is a CONTAINS */
    else
    {
        (node -> value) = CONTAINS;
        (node -> t_l) = left;
        (node -> t_r) = right;
        return node;
    }
}

#define xpc(c) fputc(c, out)
#define xpt(s) fprintf(out,s)
#define xpci(cond, c) if (cond) fputc(c, out)
#define xptr(s) fprintf(out,s); return

#define n_vals(node, val) ((node) -> value == val)
#define n_valn(node, val) ((node) -> value != val)
#define n_nbin(node) (((node) -> value) /2 != 0)

#define NL (root -> t_l)
#define NR (root -> t_r)

void n_print(FILE* out, BinNode* root)
{

    debug printf("%s : ( %s / %s )\n", val_str(root), val_str(root -> t_l), val_str(root -> t_r));

    switch(root -> value)
    {
    case RETAIN:            return;
    case IO:                xptr("[]");
    case OI:                xptr("[]!");
    case 0:                 xptr(" ZERO ");
    case 1:                 xptr(" UNUM ");
    case ERRVALUE:          xptr("  ??  ");
    case CONTAINS:

        /* Neither branch is binary or RETAIN */
        if(n_nbin(NL) && n_nbin(NR))
        {
            xpc('(');
            n_print(out, NL);
            xpc('/');
            n_print(out, NR);
            xpc(')');
            return;
        }
        /* One of them is binary or RETAIN */
        else
        {

            xpc(n_vals(NL, RETAIN) || n_vals(NR, RETAIN) ? '(' : '[');

            if (n_nbin(NL))
            {
                xpci(n_vals(NR, 1), '/');
                n_print(out, NL);
            }
            else if (n_nbin(NR))
            {
                xpci(n_valn(NL, 0), '/');
                n_print(out, NR);
            }

            xpc(n_vals(NL, RETAIN) || n_vals(NR, RETAIN) ? ')' : ']');

            if (n_nbin(NL)) 
            {
                xpci(n_vals(NR, 1), '!');
            }
            else if (n_nbin(NR))
            {
                xpci(n_vals(NL, 0), '!');
            }
            return;
        }
        break;
    }
}

#undef n_nonbin
#undef n_valn
#undef n_vals

#undef xpc
#undef xpci
#undef xpt
#undef xptr

#undef NL
#undef NR

#define rc(r,c) case c: return r;

char getNybble(char symbol)
{
    switch (symbol)
    {
        rc(0x0, '.')    rc(0x1, '!')    rc(0x2, '/')    rc(0x3, ']': case ')')
        rc(0x4, '%')    rc(0x5, '#')    rc(0x6, '>')    rc(0x7, '=')
        rc(0x8, '(')    rc(0x9, '<')    rc(0xA, ':')    rc(0xB, 'S')
        rc(0xC, '[')    rc(0xD, '*')    rc(0xE, '$')    rc(0xF, ';')
        default: return -4;
    }
}

char* val_str(BinNode* node)
{
    if (node == NULL)
        return "NULL";

    switch(node -> value)
    {
        rc("ZERO", 0)
        rc("UNUM", 1)
        rc("[]  ", IO)
        rc("[]! ", OI)
        rc("CON ", CONTAINS)
        rc("RET ", RETAIN)
        default: return "ERR";
    }
}

#undef rc