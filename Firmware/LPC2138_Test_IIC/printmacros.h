/* PRINTF Macros
 * Done to reduce malloc hits and fflush reqs 
 * (debugging aid that stuck)
 * */
#define PRINTF0(x) printidx=sprintf(printbuf,x); \
                            printbuf[printidx]='\0';      \
                           __putstr(printbuf,printidx);
#define PRINTF1(x,y1) printidx=sprintf(printbuf,x,y1); \
                               printbuf[printidx]='\0';      \
                            __putstr(printbuf,printidx);
#define PRINTF2(x,y1,y2) printidx=sprintf(printbuf,x,y1,y2); \
                                  printbuf[printidx]='\0';      \
                        __putstr(printbuf,printidx);
#define PRINTF3(x,y1,y2,y3) printidx=sprintf(printbuf,x,y1,y2,y3); \
                                     printbuf[printidx]='\0';      \
                          __putstr(printbuf,printidx);
//extern int _write(int file, char* ptr, int len);

extern int printidx;
extern char printbuf[128];
#define IAP_LOCATION 0x7ffffff1

