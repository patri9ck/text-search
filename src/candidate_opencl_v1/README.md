Kernal Code:
To check if a query is equal to a substring of the given text we
create a 2D grind of workers. On one axis we have the for e.g 10 strings that we are looking for (queries).
On the other axis we have the text positions, so if the text contains 1000 characters, we have 1000 positions minus the
length of the query.
Each worker checks if the query fits at the current text position.
As soon as one character doesn't fit we quit the loop.

Problem:

```
int pos = atomic_inc(&counts[query_idx]);
```
This leads to a bottleneck if 1000s of threads wants to increment the same counter they have to wait for each other.
And this will happen especially for words that are very common like "the" or "and". 

improvemnts from v1 to v3
- instead of each queries getting its own memory of 5 mio. slots we use a global memory of 20 million slots
we safe lots of memory
- define local workgroup size to 256 threads

v1: ca. 3.2 sekunden
v3 ca. 2.5 sekunden

idee:
use local memory to avoid reading everytime form slower global memory

static const char *kernel_source = R"raw(
__kernel void multi_search(
__global const char* book,
long book_len,
__global const char* queries,
__global const int* offsets,
__global const int* lengths,
__global int* out_char_indices,
__global int* out_query_ids,
__global int* global_counter,
int max_total_results)
{
// DEKLARATION DES LOKALEN SPEICHERS (Wichtig!)
// 256 (Workgroup Size) + 128 (maximaler Überhang für Wortlänge)
__local char local_book[384];

    int local_id = get_local_id(0);
    long global_id = get_global_id(0);
    int query_idx = get_global_id(1);

    // 1. SCHRITT: Gemeinsames Laden
    // Wir laden nur, wenn der Thread innerhalb des Buches liegt
    if (global_id < book_len) {
        local_book[local_id] = book[global_id];
    } else {
        local_book[local_id] = 0; // Padding mit Null
    }

    // Überhang laden (die ersten 128 Threads laden die nächsten 128 Bytes)
    if (local_id < 128) {
        long extra_idx = (get_group_id(0) + 1) * get_local_size(0) + local_id;
        if (extra_idx < book_len) {
            local_book[local_id + 256] = book[extra_idx];
        } else {
            local_book[local_id + 256] = 0;
        }
    }

    // Warten, bis alle 256 Threads fertig geladen haben
    barrier(CLK_LOCAL_MEM_FENCE);

    // 2. SCHRITT: Validitätscheck für die Suche
    if (global_id >= book_len) return;
    
    // Verhindern, dass wir über das Ende des Buches hinaus suchen
    if (global_id > (book_len - lengths[query_idx])) return;

    // 3. SCHRITT: Suche im lokalen Speicher
    int q_start = offsets[query_idx];
    int q_len = lengths[query_idx];

    bool match = true;
    for (int i = 0; i < q_len; i++) {
        if (local_book[local_id + i] != queries[q_start + i]) {
            match = false;
            break;
        }
    }

    if (match) {
        int pos = atomic_inc(global_counter);
        if (pos < max_total_results) {
            out_char_indices[pos] = (int)global_id;
            out_query_ids[pos] = query_idx;
        }
    }
}
)raw";