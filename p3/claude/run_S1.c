#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readline();
char* ltrim(char*);
char* rtrim(char*);

float parse_float(char*);



/*
 * Complete the 'probe' function below.
 *
 * The function is expected to return a FLOAT_ARRAY.
 * The function accepts FLOAT t as parameter.
 */

/*
 * To return the float array from the function, you should:
 *     - Store the size of the array to be returned in the result_count variable
 *     - Allocate the array statically or dynamically
 *
 * For example,
 * float* return_float_array_using_static_allocation(int* result_count) {
 *     *result_count = 5;
 *
 *     static float a[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
 *
 *     return a;
 * }
 *
 * float* return_float_array_using_dynamic_allocation(int* result_count) {
 *     *result_count = 5;
 *
 *     float* a = malloc(5 * sizeof(float));
 *
 *     for (int i = 0; i < 5; i++) {
 *         *(a + i) = i + 1.1;
 *     }
 *
 *     return a;
 * }
 *
 */
float* probe(float t, int* result_count) {
    static const float XX[51] = {-0.52678f, -0.3304f, -0.25617f, -0.18371f, -0.10308f, 0.11273f, 0.23004f, 0.45461f, -0.66802f, -0.45917f, -0.42921f, -0.3497f, -0.19817f, 0.21482f, 0.44698f, 0.89861f, -1.30982f, -0.85f, -0.64663f, -0.45323f, -0.23979f, 0.25021f, 0.52401f, 1.03037f, -1.40912f, -0.87797f, -0.721f, -0.52718f, -0.26536f, 0.25516f, 0.50601f, 1.01653f, -1.54798f, -1.04403f, -0.77278f, -0.51861f, -0.26974f, 0.26877f, 0.49455f, 0.84311f, -0.99121f, -0.48648f, -0.30847f, -0.21409f, -0.11342f, 0.11388f, 0.22958f, 0.47346f, -0.73739f, -0.50662f, -0.38971f};
    static float answer[1];
    int k = (int)floorf(t / 0.02f + 0.5f);
    if (k < 0) k = 0;
    if (k > 50) k = 50;
    answer[0] = XX[k];
    *result_count = 1;
    printf("QRT_RUN_S1 t=%.9g x=%.9g\n", (double)t, (double)answer[0]);
    return answer;
}

int main()
{
    FILE* fptr = fopen(getenv("OUTPUT_PATH"), "w");

    float t = parse_float(ltrim(rtrim(readline())));

    int result_count;
    float* result = probe(t, &result_count);

    for (int i = 0; i < result_count; i++) {
        fprintf(fptr, "%f", *(result + i));

        if (i != result_count - 1) {
            fprintf(fptr, "\n");
        }
    }

    fprintf(fptr, "\n");

    fclose(fptr);

    return 0;
}

char* readline() {
    size_t alloc_length = 1024;
    size_t data_length = 0;

    char* data = malloc(alloc_length);

    while (true) {
        char* cursor = data + data_length;
        char* line = fgets(cursor, alloc_length - data_length, stdin);

        if (!line) {
            break;
        }

        data_length += strlen(cursor);

        if (data_length < alloc_length - 1 || data[data_length - 1] == '\n') {
            break;
        }

        alloc_length <<= 1;

        data = realloc(data, alloc_length);

        if (!data) {
            data = '\0';

            break;
        }
    }

    if (data[data_length - 1] == '\n') {
        data[data_length - 1] = '\0';

        data = realloc(data, data_length);

        if (!data) {
            data = '\0';
        }
    } else {
        data = realloc(data, data_length + 1);

        if (!data) {
            data = '\0';
        } else {
            data[data_length] = '\0';
        }
    }

    return data;
}

char* ltrim(char* str) {
    if (!str) {
        return '\0';
    }

    if (!*str) {
        return str;
    }

    while (*str != '\0' && isspace(*str)) {
        str++;
    }

    return str;
}

char* rtrim(char* str) {
    if (!str) {
        return '\0';
    }

    if (!*str) {
        return str;
    }

    char* end = str + strlen(str) - 1;

    while (end >= str && isspace(*end)) {
        end--;
    }

    *(end + 1) = '\0';

    return str;
}

float parse_float(char* str) {
    char* endptr;
    float value = strtof(str, &endptr);

    if (endptr == str || *endptr != '\0') {
        exit(EXIT_FAILURE);
    }

    return value;
}
