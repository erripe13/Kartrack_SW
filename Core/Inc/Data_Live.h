#ifndef DATA_LIVE_H
#define DATA_LIVE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float vitesse; // Represents live data
} Data_Live_t;

extern Data_Live_t Data_Live; // Declare Data_Live as extern

#ifdef __cplusplus
}
#endif

#endif // DATA_LIVE_H
