/* 256 Byte circular buffer implementation
 * Done by David Wolpoff 
 * Last Modified 1 July 2006 */

#include "circbuf.h"

/* Initialize the head and tail pointer of the buffer
 * Does not zero contents */
void init_buffer(volatile buf_type* buffer)
{
  buffer->head = 0;
  buffer->tail = 0;
}

/* Inserts data into buffer. 
 * Returns 0 on success
 * Returns -1 on failure
 */
int bufinsert(volatile buf_type* buffer, unsigned int data)
{
  if(HEADTOTAIL(buffer) < BUF_SIZE-1)
  {
    buffer->head=(buffer->head+1)%BUF_SIZE;
    buffer->circle[buffer->head] = data;
    return 0;
  }
  else
  {
    return -1;
  }
}

/* Extracts a single element from the buffer 
 * Will always return a value.
 * Caller is responsible for checking if data is
 * valid. */
unsigned int bufextract(volatile buf_type* buffer)
{
  if(HEADTOTAIL(buffer) > 0)
  {
    buffer->tail=(buffer->tail+1)%BUF_SIZE;
    return buffer->circle[buffer->tail];
  }
  return 0;
}

/* Returns the number of bytes currently
 * IN USE in the buffer.
 */
int bufused(volatile buf_type* buffer)
{
  return HEADTOTAIL(buffer);
}
