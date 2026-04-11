#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


#define BLOCK_SIZE 32   //32 * 16 = 512 bytes
#define BLOCK_COUNT 16

/*
    Acest program are rolul de a implementa un pool allocator.
    Sunt prezente mai multe functionalitati.
    Capacitatea de testare : 512 bytes (este scalabil)

    Todi Tinu-Constantin
*/


typedef struct freeBlock {
    struct freeBlock* next;
}freeblock;

typedef struct freeList {
    uint8_t memory[BLOCK_SIZE * BLOCK_COUNT]; //array de 512 bytes
    freeblock* head;
    int used[BLOCK_COUNT]; // 0 = ocupat // 1 = este liber
}poolallocator;


int isValid(poolallocator* pool, void* ptr) {

    if (ptr == NULL) {
        return 0;
    }

    uint8_t* p = (uint8_t*)ptr;
    uint8_t* start = pool -> memory;
    uint8_t* end = pool -> memory + BLOCK_SIZE * BLOCK_COUNT;

    if (p < start || p >= end) {
        return 0;
    }

    size_t offset = (size_t)(p - start);

    if (offset % BLOCK_SIZE != 0) {
        return 0;
    }

    return 1;
}

int pointerToIndex(poolallocator* pool, void* ptr) {
    if (ptr == NULL || pool == NULL || !isValid(pool, ptr)) {
        exit(1);
    }

    uint8_t* start = pool -> memory;
    uint8_t* p = (uint8_t*)ptr;

    int index = (int)((p - start) / BLOCK_SIZE);
    return index;
}


void initAllocator(poolallocator* pool) {
    pool -> head = NULL;

    for (int i = 0; i < BLOCK_COUNT; i++) {
        freeblock* block = (freeblock*)(pool -> memory + i * BLOCK_SIZE);
        block -> next = pool -> head;
        pool -> head = block;
        pool -> used[i] = 1;
    }
}

void* allocate(poolallocator* pool) {
    if (pool -> head == NULL) {
        printf("Pool epuiazat.");
        return NULL;
    }


    freeblock* block = pool -> head;
    pool -> head = block -> next; //urmatorul nod devine capul listei

    int index = pointerToIndex(pool, block);
    pool -> used[index] = 0; //blocul este ocupat!

    return (void*)block;
}

void* allocateWValue(poolallocator* pool, uint8_t value) {

    void* ptr = allocate(pool);

    if (ptr == NULL) {
        return NULL;
    }

    memset(ptr, value, BLOCK_SIZE);

    return (void*)ptr;
}




void freepool(poolallocator* pool, void* ptr) {
    if (!isValid(pool, ptr)) {
        puts("Pointerul nu este valid!");
        return;
    }


    int index = pointerToIndex(pool, ptr);
    if (pool -> used[index] == 1) {
        puts("Blocul deja a fost eliberat!");
        return;
    }

    freeblock* block = (freeblock*)ptr;
    block -> next = pool -> head;
    pool -> head = block;

    pool -> used[index] = 1;
}

void countingFrees(poolallocator* pool) {

    int count = 0;
    freeblock* current = pool -> head;

    while (current != NULL) {
        count++;
        current = current -> next;
    }

    printf("Mai sunt %d blocuri libere\n", count);
}


void statistics(poolallocator* pool) {
    countingFrees(pool);

    printf("Statusul de ocupare al alocatorului : ");
    for (int i = 0; i < BLOCK_COUNT; i++) {
        printf("%d ", pool -> used[i]);
    }

    puts("\n0 ocupat, 1 liber");


    int fullCapacity = BLOCK_COUNT * BLOCK_SIZE;
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (pool -> used[i] == 0) {
            fullCapacity = fullCapacity - BLOCK_SIZE;
        }
    }
    printf("Mai sunt %d bytes disponibili.\n", fullCapacity);
}

void poolReset(poolallocator* pool) {
    initAllocator(pool);
}

void* allocate_array(poolallocator* pool, size_t elemsize , int count) {
    if (elemsize * count > BLOCK_SIZE) {
        return NULL;
    }

    return allocate(pool);
}

int main(void) {

    /// TESTING

    poolallocator pool;
    initAllocator(&pool);

    countingFrees(&pool);

    void* ptr1 = allocate(&pool);

    printf("A fost alocat blocul : %p\n", ptr1);
    void* ptr2 = allocate(&pool);
    printf("A fost alocat blocul : %p\n", ptr2);

    countingFrees(&pool);

    freepool(&pool, ptr1);

    countingFrees(&pool);

    int a = 5;

    *(int*)ptr2 = a;

    printf("Valoarea stocata la adresa %p (ptr2) este : %d\n", ptr2, *(int*)ptr2);

    freepool(&pool, ptr2);
    freepool(&pool, ptr2);
    countingFrees(&pool);

    statistics(&pool);

    int b = 10;
    void* ptr = allocate(&pool);

    *(int*)ptr = 10;

    statistics(&pool);

    freepool(&pool, ptr);

    statistics(&pool);

    poolReset(&pool);

    statistics(&pool);

    void* ptr11 = allocate(&pool);
    printf("%p -> %d\n", ptr11,*(int*)ptr11);

    *(int*)ptr11 = b;

    printf("%p -> %d\n", ptr11,*(int*)ptr11);

    statistics(&pool);

    freepool(&pool, ptr11);

    int* ptr12 = allocateWValue(&pool, 0);

    printf("%p -> %d\n", ptr12, *ptr12);

    statistics(&pool);


    int* arr = (int*)allocate_array(&pool,sizeof(int), 5);

    for (int i = 0; i < 5; i++) {
        arr[i] = i;
        printf("%d ", arr[i]);
    }
    printf("\n");
    statistics(&pool);

    freepool(&pool, arr);

    statistics(&pool);

    freepool(&pool, ptr12);
    statistics(&pool);

    return 0;
}
