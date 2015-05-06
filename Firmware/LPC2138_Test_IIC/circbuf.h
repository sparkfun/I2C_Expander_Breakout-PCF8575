/* 256 Byte circular buffer implementation 
 * Done by David Wolpoff 
 * Last Modified 1 July 2006*/

#define BUF_SIZE 256
#define HEADTOTAIL(x) ( ((x->head - x->tail)<0) ? x->head-x->tail+BUF_SIZE : x->head-x->tail)

typedef struct
{
  int  head;
  int  tail;
  unsigned int circle[BUF_SIZE];
} buf_type;

void init_buffer(volatile buf_type* buffer);

/* returns 0 on success, -1 on fail to insert */
int bufinsert(volatile buf_type* buffer, unsigned int data);

/* Always returns something. */
unsigned int bufextract(volatile buf_type* buffer);

/* Number of occupied spaces in buffer */
int bufused(volatile buf_type* buffer);

