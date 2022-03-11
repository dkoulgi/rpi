//#define DEBUG 
#if defined(DEBUG) 
#define DPRINTF(fmt, args...) printf("DEBUG: %s:%d:%s(): " fmt, \
     __FILE__, __LINE__, __func__, ##args)
#else
  #define DPRINTF(fmt, args...) /* do nothing if not defined*/
#endif
