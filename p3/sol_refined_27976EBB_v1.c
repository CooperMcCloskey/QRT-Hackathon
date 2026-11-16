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
    static const double C[5][22] = {{0.00863124921794,-0.401677429847,-0.294193055067,0.0121366089271,0.193888563091,-0.0120229591233,-0.0430760864548,0.0136895894248,-0.00620662472842,-0.019010058545,-0.0799244165511,0.0175788114041,0.0349610665109,-0.0053215039637,-0.0201202171766,0.00056693514543,0.0068348262537,0.00731992503089,-0.0042498850703,0.00324422175359,-0.0026459176807,0.00372471896814},
{-0.0330145524278,8.38528372628,3.57699610809,-0.00624380307887,0.160068698472,-0.0262083727725,0.197079735767,-0.0056084727952,0.111881325495,-0.0118982082716,0.153784645772,0.023189302896,-0.0362921581983,0.0319119094359,0.0324072804317,-0.0125966312934,-0.02958720729,0.0280603748697,-0.023442118938,0.0260028917673,0.000991095073645,-0.00906490980627},
{-0.153091907968,0.283843717013,0.220504698125,0.0706791055323,0.0536341368868,-0.0353732871597,-0.0229078487297,-0.0665921023164,-0.0474473050351,0.00903968427923,0.0111927357282,-0.0016019540059,0.00175564169541,-0.00197412207726,-0.00067171110208,0.0054289641472,0.00197143434437,-0.00174350661985,0.000472779761401,-0.001014190772,0.00140739748504,-0.00127340607933},
{0.0133472586396,0.0797066011871,-0.00756177928863,0.0136315136537,-0.00174067676805,-0.00105896138858,0.00155030634661,0.000487768900595,-9.16993906574e-05,2.65994314129e-05,-0.000240411344591,3.97551071907e-05,6.30710695151e-06,1.58891396246e-05,4.04296562005e-06,-9.65492366357e-06,-1.72716310724e-06,-1.0931855361e-05,4.42836796452e-06,-6.26893379255e-07,1.78056228577e-07,-9.35364923561e-07},
{-0.0358447785652,3.07810416512,1.19969953941,0.0474519426528,0.0716836911839,-0.0166220910909,0.0496419592159,0.0666588289455,-0.0116825820063,-0.105208332247,0.0108300896923,0.0714687865121,-6.09767073214e-05,-0.000833308312239,-0.00137933455076,0.0249416088177,-0.01492154802,-0.0193952285447,0.00968027450646,-0.0060891795614,0.00333100338211,-0.00732218611591}};
    static const int W[5][2] = {{0,0}, {1,1}, {1,2}, {2,1}, {2,2}};
    static float answer[4096];
    double z = isfinite(t) ? fmax(0.0, fmin(1.0, (double)t)) : 0;
    double basis[22], a[5] = {0};
    basis[0] = 1; basis[1] = z-.5;
    for (int k=1;k<=20;k++) basis[k+1]=cos(3.141592653589793*k*z);
    for (int j=0;j<5;j++) for(int k=0;k<22;k++) a[j]+=C[j][k]*basis[k];
    for (int i=0;i<4096;i++) {
        double u=6.283185307179586*i/4095.0, value=0;
        for (int j=0;j<5;j++) value+=a[j]*(W[j][1]==0 ? 1 : W[j][1]==1 ? cos(W[j][0]*u) : sin(W[j][0]*u));
        value*=1;
        answer[i]=isfinite(value) ? (float)fmax(-3.0,fmin(3.0,value)) : 0;
    }
    *result_count=4096;
    printf("QRT_BULK_REBUILD_27976EBB_1 t=%.9g x=%.9g\n",(double)t,(double)answer[0]);
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
