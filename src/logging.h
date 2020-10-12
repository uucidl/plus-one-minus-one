#ifndef LOGGING_H
#define LOGGING_H

#define LOG_NEW_DEFAULT_CATEGORY(x)
#define LOG_NEW_DEFAULT_SUBCATEGORY(x, y)

#define ERROR(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#define WARNING(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#define TRACE(...)
#define DEBUG(...)
#define INFO(...)

#define log_setControlString(...)

#endif
