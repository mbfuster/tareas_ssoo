#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "structs.h"


//sort sacado de http://www.anyexample.com/programming/c/qsort__sorting_array_of_strings__integers_and_structs.xml
int struct_cmp_by_time(const void *a, const void *b)
{
    const Process *ia = (Process *)a;
    const Process *ib = (Process *)b;

    return (ia->time_of_arrival, ib->time_of_arrival);

}

void sort_structs(size_t structs_len, Process** structs)
{
    /* sort array using qsort functions */
    qsort(structs, structs_len, sizeof(Process*), struct_cmp_by_time);
}
