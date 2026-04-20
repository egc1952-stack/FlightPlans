// Module: FlightPlanOrder
// Purpose: Generate all airport orderings that minimize distance while
//          respecting Beg/End precedence constraints.
// Inputs: array of airports, array of Beg/End pairs, optional starting
//         airport that is mandatory, there can be multiple ending airports in the pairings
// Output: array of valid permutations with total distances
// Constraints:
// - End(n) must appear after Beg(n)
// - If a starting airport is specified, it must be the first in the order
// - No other restrictions
// Steps:
// 1. Represent airports and constraints
// 2. Generate permutations recursively
// 3. Reject permutations that violate constraints
// 4. Calculate great circle distance for valid permutations
// 5. Return only valid permutations with distances

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_AIRPORTS 10
#define MAX_CONSTRAINTS 10
#define MAX_RESULTS 1000

typedef struct {
    char code[4];
    double lat, lon;
    int index;
} Airport;

typedef struct {
    int beg;
    int end;
} Constraint;

typedef struct {
    int airports[MAX_AIRPORTS];
    int count;
    double distance;
} Permutation;

double greatCircleDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0; // Earth radius in km
    double dlat = (lat2 - lat1) * M_PI / 180.0;
    double dlon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dlat/2)*sin(dlat/2) + cos(lat1*M_PI/180.0)*cos(lat2*M_PI/180.0)*sin(dlon/2)*sin(dlon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return R * c;
}

int isValidPermutation(Permutation* perm, Constraint* constraints, int numConstraints) {
    int position[MAX_AIRPORTS];
    memset(position, -1, sizeof(position));
    for (int i = 0; i < perm->count; i++) {
        position[perm->airports[i]] = i;
    }
    
    for (int i = 0; i < numConstraints; i++) {
        if (position[constraints[i].beg] > position[constraints[i].end]) {
            return 0;
        }
    }
    return 1;
}

void generatePermutations(Airport* airports, int numAirports, 
                         Constraint* constraints, int numConstraints,
                         Permutation* current, int used[],
                         Permutation** results, int* resultCount) {
    if (current->count == numAirports) {
        if (isValidPermutation(current, constraints, numConstraints)) {
            double total = 0.0;
            for (int k = 0; k < numAirports - 1; k++) {
                int idx1 = current->airports[k];
                int idx2 = current->airports[k + 1];
                total += greatCircleDistance(airports[idx1].lat, airports[idx1].lon,
                                             airports[idx2].lat, airports[idx2].lon);
            }
            current->distance = total;
            results[*resultCount] = malloc(sizeof(Permutation));
            memcpy(results[*resultCount], current, sizeof(Permutation));
            (*resultCount)++;
        }
        return;
    }
    
    for (int i = 0; i < numAirports; i++) {
        if (!used[i]) {
            used[i] = 1;
            current->airports[current->count++] = i;
            generatePermutations(airports, numAirports, constraints, numConstraints,
                               current, used, results, resultCount);
            current->count--;
            used[i] = 0;
        }
    }
}

int main() {
    FILE *fin = fopen("input.txt", "r");
    if (!fin) {
        printf("Error opening input.txt\n");
        return 1;
    }
    
    int numAirports;
    fscanf(fin, "%d", &numAirports);
    
    Airport airports[MAX_AIRPORTS];
    for (int i = 0; i < numAirports; i++) {
        fscanf(fin, "%s %lf %lf", airports[i].code, &airports[i].lat, &airports[i].lon);
        airports[i].index = i;
    }
    
    int numConstraints;
    fscanf(fin, "%d", &numConstraints);
    
    Constraint constraints[MAX_CONSTRAINTS];
    for (int i = 0; i < numConstraints; i++) {
        char beg[4], end[4];
        fscanf(fin, "%s %s", beg, end);
        for (int j = 0; j < numAirports; j++) {
            if (strcmp(airports[j].code, beg) == 0) constraints[i].beg = j;
            if (strcmp(airports[j].code, end) == 0) constraints[i].end = j;
        }
    }
    
    int startIdx = -1;
    char start[4];
    if (fscanf(fin, "%s", start) == 1) {
        for (int j = 0; j < numAirports; j++) {
            if (strcmp(airports[j].code, start) == 0) startIdx = j;
        }
    }
    
    fclose(fin);
    
    Permutation* results[MAX_RESULTS];
    int resultCount = 0;
    Permutation current;
    int used[MAX_AIRPORTS] = {0};
    
    if (startIdx != -1) {
        current.airports[0] = startIdx;
        current.count = 1;
        used[startIdx] = 1;
        generatePermutations(airports, numAirports, constraints, numConstraints,
                           &current, used, results, &resultCount);
    } else {
        current.count = 0;
        memset(used, 0, sizeof(used));
        generatePermutations(airports, numAirports, constraints, numConstraints,
                           &current, used, results, &resultCount);
    }
    
    FILE *fout = fopen("output.txt", "w");
    fprintf(fout, "%d\n", resultCount);
    for (int i = 0; i < resultCount; i++) {
        for (int j = 0; j < results[i]->count; j++) {
            fprintf(fout, "%s ", airports[results[i]->airports[j]].code);
        }
        fprintf(fout, "%.2f\n", results[i]->distance);
        free(results[i]);
    }
    fclose(fout);
    
    return 0;
}


