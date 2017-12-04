
struct gl_data;

typedef struct renderer {
    bool    alive;
    size_t  bufsize_request, rate_request, samplesize_request;
    char*   audio_source_request;
    struct gl_data* gl;
} renderer;

struct renderer* rd_new(const char** paths, const char* entry, const char* force_mod);
void             rd_update(struct renderer*, float* lb, float* rb, size_t bsz, bool modified);
void             rd_destroy(struct renderer*);
void             rd_time(struct renderer*);
void*            rd_get_impl_window(struct renderer*);
