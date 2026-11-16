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
    static const float C0[51] = {0.01438f, 0.01171f, 0.01727f, 0.03622f, 0.06462f, 0.06861f, 0.04569f, 0.01928f, -0.02296f, -0.08164f, -0.14906f, -0.21212f, -0.26332f, -0.30135f, -0.30296f, -0.28266f, -0.25652f, -0.21212f, -0.16188f, -0.14756f, -0.19486f, -0.26762f, -0.32247f, -0.30653f, -0.20671f, -0.06782f, 0.08899f, 0.19864f, 0.24513f, 0.27228f, 0.25424f, 0.22137f, 0.21385f, 0.20897f, 0.24130f, 0.31467f, 0.34638f, 0.32712f, 0.29843f, 0.26244f, 0.20970f, 0.10096f, 0.00326f, -0.00851f, 0.01227f, 0.01904f, -0.01974f, -0.05511f, -0.02967f, 0.02956f, 0.06946f};
    static const float A1[51] = {0.02029f, 0.03561f, 0.01338f, -0.01013f, -0.03396f, -0.10994f, -0.14070f, -0.06698f, 0.02195f, 0.09505f, 0.22275f, 0.28841f, 0.27056f, 0.29463f, 0.28700f, 0.25897f, 0.25709f, 0.24380f, 0.22016f, 0.17852f, 0.16057f, 0.21455f, 0.27075f, 0.23523f, 0.11140f, -0.05120f, -0.19232f, -0.31705f, -0.38610f, -0.34102f, -0.28723f, -0.20994f, -0.09854f, -0.14169f, -0.32699f, -0.43118f, -0.42459f, -0.36142f, -0.26641f, -0.23716f, -0.21531f, -0.13245f, -0.01625f, 0.09703f, 0.08370f, -0.02440f, -0.09449f, -0.19778f, -0.27404f, -0.12115f, 0.06243f};
    static const float B1[51] = {-0.10748f, -0.08047f, -0.04481f, -0.04276f, -0.03657f, 0.01046f, 0.10736f, 0.18474f, 0.13583f, 0.08116f, 0.08120f, 0.02798f, -0.04158f, -0.08134f, -0.14871f, -0.21421f, -0.23878f, -0.31184f, -0.34896f, -0.24488f, -0.17496f, -0.14944f, -0.08352f, -0.12100f, -0.22199f, -0.20662f, -0.13902f, -0.17873f, -0.25079f, -0.22795f, -0.22663f, -0.29182f, -0.28382f, -0.20493f, -0.12483f, -0.09826f, -0.14157f, -0.11081f, -0.08307f, -0.18647f, -0.23723f, -0.18142f, -0.11145f, -0.12847f, -0.26148f, -0.30160f, -0.20771f, -0.15913f, -0.19079f, -0.23828f, -0.27670f};
    static const float A2[51] = {-0.00000f, -0.00000f, -0.00000f, 0.00000f, 0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, 0.00000f, 0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f, -0.00000f, -0.00000f, -0.00000f, -0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00000f};
    static const float B2[51] = {-0.12601f, -0.10879f, -0.08047f, -0.04700f, -0.05212f, -0.06573f, -0.05542f, -0.01389f, 0.03581f, 0.01144f, -0.03571f, -0.06079f, -0.09625f, -0.10669f, -0.09113f, -0.05461f, 0.01826f, 0.07820f, 0.09330f, 0.09763f, 0.09319f, 0.06026f, 0.02892f, -0.05937f, -0.23232f, -0.32431f, -0.25936f, -0.16450f, -0.10854f, -0.01862f, 0.07056f, 0.04857f, -0.00892f, -0.01730f, -0.02281f, -0.03976f, -0.03144f, -0.05898f, -0.12407f, -0.10016f, -0.01048f, 0.04331f, 0.06434f, 0.07276f, 0.01553f, -0.02586f, 0.02469f, 0.08999f, 0.10723f, 0.11881f, 0.16258f};
    static float answer[4096];
    int k = (int)floorf(t / 0.02f + 0.5f);
    if (k < 0) k = 0;
    if (k > 50) k = 50;
    for (int i = 0; i < 4096; ++i) {
        double u = (double)i / 4095.0;
        double w = 6.283185307179586 * u;
        answer[i] = (float)(C0[k] + A1[k] * cos(w) + B1[k] * sin(w) + A2[k] * cos(2 * w) + B2[k] * sin(2 * w));
    }
    *result_count = 4096;
    printf("QRT_RUN_WAVES t=%.9g x=%.9g\n", (double)t, (double)answer[0]);
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
