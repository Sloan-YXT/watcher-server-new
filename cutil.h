#ifndef CUTIL
#define CUTIL
#include <ctime>
char *trim(char *);
char *trim(char *,char);
char *rtrim(char *,char);
char *ltrin(char *,char);
void rmAll(const char *path);
void rm(const char *path);
#define FDEBUG(F, X, ...)                   \
    do                                      \
    {                                       \
        \  
    FILE *p = fopen(F, "a");                \
        fprintf(p, "in line %d", __LINE__); \
        fprintf(p, X, ##__VA_ARGS__);       \
        fprintf(p, "\n");                   \
        fclose(p);                          \
    } while (0)
#define FTDEBUG(F, T, X, ...)                              \
    do                                                     \
    {                                                      \
        \  
    FILE *p = fopen(F, "a");                               \
    struct tm t;\
    char stTmp[32];\
    time_t now;\
    time(&now);\
    localtime_r(&now, &t);\
    asctime_r(&t,stTmp);\
        fprintf(p, "\n\n(on %s)<%s>\n\nin line %d",stTmp,T, __LINE__); \
                                                           \
        fprintf(p, X, ##__VA_ARGS__);                      \
        fprintf(p, "\n\n<\\%s>\n\n",\ 
        T);                                                \
        fclose(p);                                         \
    } while (0)
#ifdef DEBUG
#define DEBUG(X,...)                              \
    do                                        \
    {                                         \
                                              \
        char msg_[256];                         \
        sprintf(msg_,"debug:line%d;;;;;",__LINE__);\
        strcat(msg_,X);\
        strcat(msg_,"\n");\
        printf(msg_,##__VA_ARGS__); \
    } while (0)
#define DEBUG_MODE 1
#else
#define DEBUG(X)
#define FDEBUG(F, X, ...)
#define FTDEBUG(F, T, X, ...)
#endif
#endif