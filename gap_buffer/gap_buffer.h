
typedef struct gap_buffer_t  {
    char *buff;
} GB_t;

/* Inserts character c to the cursor position */
void insert(GB_t *GB, char c);

/* Deletes the character at the cursor position */
void delete(GB_t *GB);
