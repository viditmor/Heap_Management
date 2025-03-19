#include <stdio.h>
#include <stdlib.h>

// Structure representing a memory segment
typedef struct MemSegment {
    int startAddr;  // Beginning address of the segment
    int length;     // Size of the segment in bytes
    void* memPtr;   // Pointer to allocated space
    struct MemSegment* next; // Pointer to the next segment in the list
} MemSegment;

void* simulatedHeap;          // Simulated heap storage
MemSegment* availableSegs;    // Linked list of free segments
MemSegment* occupiedSegs;     // Linked list of allocated segments

// Function declarations
void showSegmentList(MemSegment* list);
void showMemoryState();
void setupHeap();
void releaseHeap();
void* requestMemory(int size);
void combineFreeSegments();
void releaseMemory(void* memPtr);

// Function to display segments in a given list (either free or occupied)
void showSegmentList(MemSegment* list) {
    MemSegment* ptr = list;
    printf("Address\tSize\n");
    while (ptr != NULL) {
        printf("%d\t%d\n", ptr->startAddr, ptr->length);
        ptr = ptr->next;
    }
    printf("\n");
}

// Function to display both free and allocated memory segments
void showMemoryState() {
    printf("\nFree Memory Segments\n");
    showSegmentList(availableSegs);
    printf("Allocated Memory Segments\n");
    showSegmentList(occupiedSegs);
}

// Function to initialize the heap with a single free segment
void setupHeap() {
    simulatedHeap = malloc(1024); // Creating a 1024-byte simulated heap
    if (!simulatedHeap) {
        printf("Error: Heap allocation failed!\n");
        exit(1);
    }

    // Setting up the initial free segment
    availableSegs = (MemSegment*)malloc(sizeof(MemSegment));
    if (!availableSegs) {
        printf("Error: Memory segment allocation failed!\n");
        free(simulatedHeap);
        exit(1);
    }

    occupiedSegs = NULL;      // Initially, no memory is allocated
    availableSegs->next = NULL;   // Single free segment initially
    availableSegs->startAddr = 0; // Initial address
    availableSegs->length = 1024; // Full heap size
    availableSegs->memPtr = NULL; // No actual memory assigned yet
}

// Function to free memory before exiting
void releaseHeap() {
    free(simulatedHeap);  // Free the heap storage

    // Free all available segments
    MemSegment* ptr = availableSegs;
    while (ptr != NULL) {
        MemSegment* temp = ptr;
        ptr = ptr->next;
        free(temp);
    }

    // Free all allocated segments
    ptr = occupiedSegs;
    while (ptr != NULL) {
        MemSegment* temp = ptr;
        ptr = ptr->next;
        free(temp);
    }
}

// Function to allocate memory using the First-Fit method
void* requestMemory(int size) {
    if (size <= 0) {
        printf("Error: Invalid memory request size!\n");
        return NULL;
    }
    
    MemSegment* ptr = availableSegs, * prev = NULL;

    // Find a free segment large enough to accommodate the request
    while (ptr != NULL && size > ptr->length) {
        prev = ptr;
        ptr = ptr->next;
    }

    // If no suitable segment is found, return NULL
    if (ptr == NULL) return NULL;

    // Create a new allocated segment
    MemSegment* newSegment = (MemSegment*)malloc(sizeof(MemSegment));
    if (!newSegment) {
        printf("Error: Allocation failed!\n");
        return NULL;
    }
    
    // Assign attributes to the new segment
    newSegment->length = size;
    newSegment->memPtr = (char*)simulatedHeap + ptr->startAddr;
    newSegment->startAddr = ptr->startAddr;
    newSegment->next = NULL;

    // Update the free segment’s start address and size
    ptr->startAddr += size;
    ptr->length -= size;

    // If the free segment is fully used, remove it from the list
    if (ptr->length == 0) {
        if (prev == NULL) {
            availableSegs = ptr->next;
        } else {
            prev->next = ptr->next;
        }
        free(ptr);
    }

    // Add the allocated segment to the occupied list
    if (occupiedSegs == NULL) {
        occupiedSegs = newSegment;
    } else {
        MemSegment* temp = occupiedSegs;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newSegment;
    }

    return newSegment->memPtr;
}

// Function to merge adjacent free segments to reduce fragmentation
void combineFreeSegments() {
    MemSegment* ptr = availableSegs;
    while (ptr != NULL && ptr->next != NULL) {
        // Merge adjacent segments
        if (ptr->startAddr + ptr->length == ptr->next->startAddr) {
            ptr->length += ptr->next->length; // Extend segment size
            MemSegment* temp = ptr->next;
            ptr->next = temp->next;
            free(temp); // Remove merged segment
        } else {
            ptr = ptr->next;
        }
    }
}

// Function to free allocated memory
void releaseMemory(void* memPtr) {
    if (!memPtr) {
        printf("Error: Null pointer cannot be freed!\n");
        return;
    }

    MemSegment* ptr1 = occupiedSegs, *prev1 = NULL;

    // Locate the segment in the allocated list
    while (ptr1 != NULL && ptr1->memPtr != memPtr) {
        prev1 = ptr1;
        ptr1 = ptr1->next;
    }

    // If segment not found, show an error
    if (ptr1 == NULL) {
        printf("Error: Attempted to free unallocated memory!\n");
        return;
    }

    // Remove the segment from the allocated list
    if (prev1 == NULL) {
        occupiedSegs = ptr1->next;
    } else {
        prev1->next = ptr1->next;
    }

    // Reinsert the freed segment into the free list
    MemSegment* prev = NULL, *curr = availableSegs;

    while (curr != NULL && curr->startAddr < ptr1->startAddr) {
        prev = curr;
        curr = curr->next;
    }

    ptr1->next = curr;
    if (prev == NULL) {
        availableSegs = ptr1;
    } else {
        prev->next = ptr1;
    }

    // Merge adjacent free segments
    combineFreeSegments();
}

// Main function to test memory management
int main() {
    setupHeap();
    void* allocatedMem[20];
    int count = 0;
    int option;

    do {
        printf("\n1. Allocate Memory\n2. Free Memory\n3. Display Memory\n4. Exit\n");
        scanf("%d", &option);

        if (option == 1) {
            int size;
            printf("Enter size: ");
            scanf("%d", &size);
            allocatedMem[count] = requestMemory(size);
            if (allocatedMem[count]) {
                printf("Memory allocated at index %d\n", count++);
            } else {
                printf("Allocation failed.\n");
            }
        } else if (option == 2) {
            int index;
            printf("Enter index: ");
            scanf("%d", &index);
            if (index >= 0 && index < count) {
                releaseMemory(allocatedMem[index]);
                allocatedMem[index] = NULL;
            } else {
                printf("Invalid index.\n");
            }
        } else if (option == 3) {
            showMemoryState();
        }
    } while (option != 4);

    releaseHeap();
    return 0;
}
