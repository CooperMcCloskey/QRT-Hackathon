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
    static const double C[9][22] = {{0.0097854813365,0.83613970226,0.206274058388,0.0134077398103,0.261501294269,-0.0163010425723,-0.0193331736238,0.0157231761704,-0.00114057957119,-0.0148308994775,-0.068332252607,0.0120120055886,0.0485293557807,-0.00797238210976,-0.011604606146,0.00120908903627,0.0025602909045,0.00204683871468,-0.00468820871931,0.00175470735164,-0.00157058531176,0.00349579043452},
{-0.0337909301021,8.46611400403,3.61196452493,-0.00679655377611,0.162167286237,-0.0266558079329,0.197599033175,-0.00519821535067,0.113857122709,-0.0129088763364,0.154989030177,0.0253592787141,-0.0355148602568,0.0315228093429,0.0341954247245,-0.0130049000936,-0.0284091295622,0.0281711112412,-0.0238226255986,0.0256650215039,0.00138388073535,-0.0088573017317},
{-0.153290721577,0.278892214992,0.218792901781,0.0703329218159,0.0542056243706,-0.0353721030123,-0.023194254085,-0.0667886442046,-0.0472500274415,0.00925963148967,0.0111338011353,-0.00165905201398,0.0015500222028,-0.00209576003251,-0.000471387531442,0.00545800532537,0.00182075030847,-0.00166434099276,0.000458560631306,-0.000993987704023,0.00141465730341,-0.00124723567573},
{0.00593997303726,0.733191020576,0.259088253273,0.0188561840181,0.0332882798916,-0.00391971455125,0.055974630339,0.0255583796701,-0.00271162254793,-0.001648621934,-0.0314086653942,0.00326304900255,0.00569126647136,0.00410946197798,-0.00121676581178,-0.0010850899224,-0.00218320157075,-0.00187684071505,0.000723946483665,0.000251189163516,-0.000604223679223,-9.08872794641e-05},
{-0.0383072101574,0.714780701333,0.250611579635,0.0480422589795,-0.0407304299481,-0.00650762124612,0.00850217239614,0.0634777129563,-0.0378463149038,-0.0983526242971,0.00928977896587,0.0597458672129,0.00495717548148,-0.0101728750523,-0.00936102424764,0.014107707098,-0.00921264685228,-0.0201811081327,0.00626959368563,-0.000477280175358,0.00201113785795,-0.00897281135935},
{-0.0217059640335,-2.58316657059,-1.00926256001,0.0576486436362,0.020449894435,0.0247180748195,-0.137016181811,-0.0594576914974,0.0403516781762,-0.0407885393931,0.0132957762472,-0.0201222970145,0.0149576619889,-0.00369176838142,0.00215164799708,-0.00142889183146,-0.00288532225365,-0.000382237648858,-0.00256281323482,-0.00160002482836,0.00188329631599,-0.000271378234843},
{-0.00391812661124,-3.14659675734,-1.29731896324,-0.014133899691,-0.121643827941,0.12753557245,-0.0175743489285,-0.131388169075,-0.0179579319144,0.108788916177,-0.0214759092478,-0.0438844506239,0.00648365290526,-0.012181177177,0.0333544978477,-0.0241544792016,0.00621619227864,0.00317602763935,-0.00737775370757,0.00103539820056,0.00429990526673,0.00204766636373},
{0.00536501851777,0.0360315548249,-0.0196333570874,0.00597470884312,0.00143523894367,0.00102483627069,-0.00118936329094,5.37949910585e-05,0.000370760557019,-5.76141344278e-05,0.000177981345057,3.39818061252e-06,-7.07486126842e-06,3.41480640789e-05,4.99658217002e-05,-2.71082032434e-05,-7.92196337581e-06,-1.51496857377e-05,6.88083647326e-06,1.53185956792e-06,-7.71474121908e-06,-5.28553114137e-06},
{-0.000801816729328,-0.089664157097,0.0115721690493,-0.0796522064285,0.0141502668838,0.000514109117077,-0.00349068433744,0.00345978360286,-0.00114248890189,-0.000531964545866,1.28968754751e-05,0.000303019550738,-0.000263405908744,8.97500761831e-05,0.000154642777343,0.000131461007224,6.95001168678e-06,1.48318087292e-05,-8.18413730651e-06,7.36717773006e-07,5.82068179575e-06,-3.70523711368e-06}};
    static const int W[9][2] = {{0,0}, {1,1}, {1,2}, {2,1}, {2,2}, {3,1}, {3,2}, {4,1}, {4,2}};
    static float answer[4096];
    double z = isfinite(t) ? fmax(0.0, fmin(1.0, (double)t)) : 0;
    static const double TARGET[51] = {0.0148,0.0131,0.014,0.0162,0.0204,0.0244,0.0254,0.0248,0.0238,0.0253,0.0393,0.0587,0.0754,0.0886,0.0959,0.0969,0.0915,0.0867,0.0892,0.0986,0.1104,0.1202,0.1318,0.1274,0.1059,0.0925,0.1109,0.1334,0.1352,0.125,0.1229,0.124,0.1278,0.1308,0.1274,0.1291,0.1397,0.1387,0.1174,0.0853,0.0524,0.0284,0.0203,0.022,0.0247,0.0249,0.0253,0.0269,0.029,0.0308,0.0324};
    double basis[22], a[9] = {0};
    basis[0] = 1; basis[1] = z-.5;
    for (int k=1;k<=20;k++) basis[k+1]=cos(3.141592653589793*k*z);
    for (int j=0;j<9;j++) for(int k=0;k<22;k++) a[j]+=C[j][k]*basis[k];
    double at=z*50.0;
    int lo=(int)floor(at), hi=lo<50 ? lo+1 : 50;
    double target=TARGET[lo]+(at-lo)*(TARGET[hi]-TARGET[lo]);
    double energy=0;
    for(int j=0;j<9;j++) energy+=a[j]*a[j]*(W[j][1]==0 ? 1.0 : .5);
    double scale=1.25;
    if(energy*scale*scale>2*target && energy>0) scale=sqrt(2*target/energy);
    for (int i=0;i<4096;i++) {
        double u=6.283185307179586*i/4095.0, value=0;
        for (int j=0;j<9;j++) value+=a[j]*(W[j][1]==0 ? 1 : W[j][1]==1 ? cos(W[j][0]*u) : sin(W[j][0]*u));
        value*=scale;
        answer[i]=isfinite(value) ? (float)fmax(-3.0,fmin(3.0,value)) : 0;
    }
    *result_count=4096;
    printf("QRT_BULK_REBUILD_E701CD73_2 t=%.9g x=%.9g\n",(double)t,(double)answer[0]);
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
