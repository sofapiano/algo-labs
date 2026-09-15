#include "draw.h"

int main() {  
    printf("Start example of creation point:\n");
    struct Point p = createPoint(13, 24);
    printf("x=%d, y=%d\n\n", p.x, p.y);

    printf("Creating simple list\n");
    struct LinkedList *list = newList(p);

    appendItem(list, createPoint(15, 26));
    appendItem(list, createPoint(17, 30));

    printf("List points:\n"); 
    printList(list);

    appendItem(list, createPoint(132, 126));
    appendItem(list, createPoint(17, 30000));

    printf("Number of points\n");

    struct LinkedList *current = list;
    int count = 0;

    while (current != NULL) {
        count++;
        current = current -> next;
    } ;

    printf("Total number of points: %d", count); 


    freeList(list);
    printf("\nAll memory freed.\n");

    return 0;
}
