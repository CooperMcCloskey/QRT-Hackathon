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
    static const double C[9][22] = {{0.00863124921794,-0.401677429847,-0.294193055067,0.0121366089271,0.193888563091,-0.0120229591233,-0.0430760864548,0.0136895894248,-0.00620662472842,-0.019010058545,-0.0799244165511,0.0175788114041,0.0349610665109,-0.0053215039637,-0.0201202171766,0.00056693514543,0.0068348262537,0.00731992503089,-0.0042498850703,0.00324422175359,-0.0026459176807,0.00372471896814},
{-0.0330145524278,8.38528372628,3.57699610809,-0.00624380307887,0.160068698472,-0.0262083727725,0.197079735767,-0.0056084727952,0.111881325495,-0.0118982082716,0.153784645772,0.023189302896,-0.0362921581983,0.0319119094359,0.0324072804317,-0.0125966312934,-0.02958720729,0.0280603748697,-0.023442118938,0.0260028917673,0.000991095073645,-0.00906490980627},
{-0.153091907968,0.283843717013,0.220504698125,0.0706791055323,0.0536341368868,-0.0353732871597,-0.0229078487297,-0.0665921023164,-0.0474473050351,0.00903968427923,0.0111927357282,-0.0016019540059,0.00175564169541,-0.00197412207726,-0.00067171110208,0.0054289641472,0.00197143434437,-0.00174350661985,0.000472779761401,-0.001014190772,0.00140739748504,-0.00127340607933},
{-0.00577750517564,-0.0563854978803,-0.0538468503897,0.00624534900063,0.00404755196816,0.00113603100525,0.00178296670045,0.000555005816469,-7.96391572059e-05,-6.69601631562e-05,-0.000177558385948,-1.19415689871e-05,5.02076212805e-05,2.27550498742e-05,-2.11550564537e-05,4.79504949377e-06,-1.99104048131e-05,8.2459355018e-07,-1.39563120166e-06,3.53454939854e-06,-6.1448196529e-06,1.14607152137e-06},
{-0.0422333327366,-0.0131361766586,-0.0315014353675,0.0491526414454,-0.079207381966,0.00522492684174,-0.00590096395758,0.0494558629987,-0.0394405088965,-0.0578434741613,0.0105626779191,0.0196703999483,0.00701315733007,-0.0075062581706,-0.00365241522747,-0.00054112635383,0.000422862687383,-0.00407130140382,0.00062094502906,0.00102336391823,0.000296047621147,-0.00154573615547},
{-0.0217059640335,-2.58316657059,-1.00926256001,0.0576486436362,0.020449894435,0.0247180748195,-0.137016181811,-0.0594576914974,0.0403516781762,-0.0407885393931,0.0132957762472,-0.0201222970145,0.0149576619889,-0.00369176838142,0.00215164799708,-0.00142889183146,-0.00288532225365,-0.000382237648858,-0.00256281323482,-0.00160002482836,0.00188329631599,-0.000271378234843},
{-0.00391812661124,-3.14659675734,-1.29731896324,-0.014133899691,-0.121643827941,0.12753557245,-0.0175743489285,-0.131388169075,-0.0179579319144,0.108788916177,-0.0214759092478,-0.0438844506239,0.00648365290526,-0.012181177177,0.0333544978477,-0.0241544792016,0.00621619227864,0.00317602763935,-0.00737775370757,0.00103539820056,0.00429990526673,0.00204766636373},
{0.00536501851777,0.0360315548249,-0.0196333570874,0.00597470884312,0.00143523894367,0.00102483627069,-0.00118936329094,5.37949910585e-05,0.000370760557019,-5.76141344278e-05,0.000177981345057,3.39818061252e-06,-7.07486126842e-06,3.41480640789e-05,4.99658217002e-05,-2.71082032434e-05,-7.92196337581e-06,-1.51496857377e-05,6.88083647326e-06,1.53185956792e-06,-7.71474121908e-06,-5.28553114137e-06},
{-0.000801816729328,-0.089664157097,0.0115721690493,-0.0796522064285,0.0141502668838,0.000514109117077,-0.00349068433744,0.00345978360286,-0.00114248890189,-0.000531964545866,1.28968754751e-05,0.000303019550738,-0.000263405908744,8.97500761831e-05,0.000154642777343,0.000131461007224,6.95001168678e-06,1.48318087292e-05,-8.18413730651e-06,7.36717773006e-07,5.82068179575e-06,-3.70523711368e-06}};
    static const int W[9][2] = {{0,0}, {1,1}, {1,2}, {2,1}, {2,2}, {3,1}, {3,2}, {4,1}, {4,2}};
    static float answer[4096];
    double z = isfinite(t) ? fmax(0.0, fmin(1.0, (double)t)) : 0;
    double basis[22], a[9] = {0};
    basis[0] = 1; basis[1] = z-.5;
    for (int k=1;k<=20;k++) basis[k+1]=cos(3.141592653589793*k*z);
    for (int j=0;j<9;j++) for(int k=0;k<22;k++) a[j]+=C[j][k]*basis[k];
    for (int i=0;i<4096;i++) {
        double u=6.283185307179586*i/4095.0, value=0;
        for (int j=0;j<9;j++) value+=a[j]*(W[j][1]==0 ? 1 : W[j][1]==1 ? cos(W[j][0]*u) : sin(W[j][0]*u));
        value*=1.25;
        answer[i]=isfinite(value) ? (float)fmax(-3.0,fmin(3.0,value)) : 0;
    }
    *result_count=4096;
    printf("QRT_BULK_REBUILD_F5A0FE5D_2 t=%.9g x=%.9g\n",(double)t,(double)answer[0]);
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
