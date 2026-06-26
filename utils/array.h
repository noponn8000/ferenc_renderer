#define darray(T) struct { \
    T *items; \
    size_t count;\
    size_t capacity;\
}

#define dappend(xs, x)\
    do {\
        if (xs.count >= xs.capacity) {\
            if (xs.capacity == 0) xs.capacity = 4;\
            else xs.capacity *= 2;\
            xs.items = realloc(xs.items, xs.capacity * sizeof(*xs.items));\
        }\
        xs.items[xs.count++] = x;\
    } while (0);

#define dremove(xs)\
    do {\
        if (xs.count > 0) {\
            xs.count--;\
            if (xs.count >= 4 && xs.count * 2 <= xs.capacity) {\
                xs.capacity /= 2;\
                xs.items = realloc(xs.items, xs.capacity * sizeof(*xs.items));\
            }\
        }\
    } while (0);
