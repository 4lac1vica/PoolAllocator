#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>


#define BLOCK_SIZE 32   //32 * 16 = 512 bytes
#define BLOCK_COUNT 16
#define FULLCAP 512


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
    printf("Mai sunt %d bytes disponibili.", fullCapacity);
}

int main(void) {

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


    return 0;
}
