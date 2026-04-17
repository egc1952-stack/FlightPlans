// Module: FlightPlanOrder
// Purpose: Generate all airport orderings that minimize distance while
//          respecting Beg/End precedence constraints.
// Inputs: array of airports, array of Beg/End pairs, optional starting
//         airport that is mandatory, there can be multiple ending airports in the pairings
// Output: array of valid permutations
// Constraints:
// - End(n) must appear after Beg(n)
// - If a starting airport is specified, it must be the first in the order
// - No other restrictions
// Steps:
// 1. Represent airports and constraints
// 2. Generate permutations recursively
// 3. Reject permutations that violate constraints
// 4. Return only valid permutations

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_AIRPORTS 10
#define MAX_CONSTRAINTS 10

typedef struct {
    char code[4];
    int index;
} Airport;

typedef struct {
    int beg;
    int end;
} Constraint;

typedef struct {
    int airports[MAX_AIRPORTS];
    int count;
} Permutation;

int isValidPermutation(Permutation* perm, Constraint* constraints, int numConstraints) {
    int position[MAX_AIRPORTS];
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


