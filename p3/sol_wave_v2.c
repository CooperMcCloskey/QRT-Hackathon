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
    static const double C[5][22] = {
        {0.00863124921794, -0.401677429847, -0.294193055067, 0.0121366089271, 0.193888563091, -0.0120229591233, -0.0430760864548, 0.0136895894248, -0.00620662472842, -0.019010058545, -0.0799244165511, 0.0175788114041, 0.0349610665109, -0.0053215039637, -0.0201202171766, 0.00056693514543, 0.0068348262537, 0.00731992503089, -0.0042498850703, 0.00324422175359, -0.0026459176807, 0.00372471896814},
        {-0.0330226126213, 8.38733091469, 3.57786939816, -0.00624532744486, 0.160107777744, -0.026214771301, 0.197127850937, -0.00560984205125, 0.111908640271, -0.0119011131076, 0.153822190851, 0.0231949643469, -0.0363010185885, 0.0319197004294, 0.0324151923654, -0.0125997066428, -0.0295944307293, 0.0280672255471, -0.0234478421116, 0.0260092401296, 0.000991337040216, -0.00906712291902},
        {-0.153054532014, 0.283774419231, 0.22045086397, 0.0706618498913, 0.0536210426151, -0.0353646511032, -0.0229022559932, -0.0665758444789, -0.0474357212204, 0.00903747732506, 0.0111900031267, -0.00160156290384, 0.00175521307195, -0.00197364011387, -0.000671547110111, 0.0054276387165, 0.00197095303715, -0.00174308095905, 0.000472664336655, -0.00101394316683, 0.00140705388214, -0.00127309518917},
        {0.0133505172477, 0.0797260608065, -0.00756362542615, 0.0136348416599, -0.00174110173796, -0.00105921992407, 0.00155068483937, 0.000487887984799, -9.1721778204e-05, 2.66059254147e-05, -0.000240470038767, 3.97648130274e-05, 6.30864677254e-06, 1.58930188091e-05, 4.04395267221e-06, -9.65728082267e-06, -1.72758477792e-06, -1.0934524271e-05, 4.42944910904e-06, -6.27046429396e-07, 1.78099699336e-07, -9.35593284138e-07},
        {-0.0358360273985, 3.07735267484, 1.19940664401, 0.0474403577058, 0.0716661902828, -0.0166180329632, 0.049629839597, 0.0666425548173, -0.0116797298134, -0.105182646619, 0.0108274456274, 0.0714513380779, -6.09618204295e-05, -0.000833104867827, -0.00137899779916, 0.0249355195577, -0.014917905064, -0.0193904933815, 0.00967791115819, -0.00608769294529, 0.00333019014886, -0.00732039847282}
    };
    static float answer[4096];
    double z = isfinite(t) ? fmax(0.0, fmin(1.0, (double)t)) : 0.0;
    double basis[22], a[5] = {0};
    basis[0] = 1.0; basis[1] = z - 0.5;
    for (int k = 1; k <= 20; ++k) basis[k+1] = cos(3.141592653589793 * k * z);
    for (int j = 0; j < 5; ++j)
        for (int k = 0; k < 22; ++k) a[j] += C[j][k] * basis[k];
    for (int i = 0; i < 4096; ++i) {
        double u = 6.283185307179586 * i / 4095.0;
        double value = (a[0] + a[1]*cos(u) + a[2]*sin(u) + a[3]*cos(2*u) + a[4]*sin(2*u)) * 0.789889415482;
        answer[i] = isfinite(value) ? (float)fmax(-2.0, fmin(2.0, value)) : 0.0f;
    }
    *result_count = 4096;
    printf("QRT_BULK_MODEL2 t=%.9g x=%.9g\n", (double)t, (double)answer[0]);
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
