// Module: FlightPlanOrder
// Purpose: Generate all airport orderings that minimize distance while
//          respecting Beg/End precedence constraints.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_AIRPORTS 10
#define MAX_CONSTRAINTS 10

typedef struct {
    char code[5];
    double lat, lon;
} Airport;

typedef struct {
    char beg[5];
    char end[5];
} Constraint;

typedef struct {
    int airports[MAX_AIRPORTS];
    int count;
    double distance;
} Permutation;

double greatCircleDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 3440.0;
    double dlat = (lat2 - lat1) * M_PI / 180.0;
    double dlon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dlat / 2.0) * sin(dlat / 2.0) +
               cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
               sin(dlon / 2.0) * sin(dlon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return R * c;
}

int findAirportIndex(Airport *airports, int numAirports, const char *code) {
    for (int i = 0; i < numAirports; i++) {
        if (strcmp(airports[i].code, code) == 0) return i;
    }
    return -1;
}

int isValidRoute(Airport *airports, int route[], int len,
                 Constraint *constraints, int numConstraints, int forcedStart) {
    if (forcedStart != -1 && len > 0 && route[0] != forcedStart) return 0;

    for (int i = 0; i < numConstraints; i++) {
        int firstBeg = -1;
        int lastEnd = -1;

        for (int j = 0; j < len; j++) {
            if (strcmp(airports[route[j]].code, constraints[i].beg) == 0 && firstBeg == -1) {
                firstBeg = j;
            }
            if (strcmp(airports[route[j]].code, constraints[i].end) == 0) {
                lastEnd = j;
            }
        }

        if (firstBeg != -1 && lastEnd != -1 && firstBeg > lastEnd) return 0;
    }

    return 1;
}

void evaluateRoute(Airport *airports, Constraint *constraints, int numConstraints,
                   int route[], int len, Permutation *best, int *hasBest,
                   int forcedStart, long long *validRouteCount) {
    if (!isValidRoute(airports, route, len, constraints, numConstraints, forcedStart)) return;
    (*validRouteCount)++;

    double total = 0.0;
    for (int i = 0; i < len - 1; i++) {
        int a = route[i];
        int b = route[i + 1];
        total += greatCircleDistance(airports[a].lat, airports[a].lon,
                                    airports[b].lat, airports[b].lon);
    }

    if (!(*hasBest) || total < best->distance) {
        best->count = len;
        for (int i = 0; i < len; i++) best->airports[i] = route[i];
        best->distance = total;
        *hasBest = 1;
    }
}

void searchRoutes(Airport *airports, int numAirports,
                 Constraint *constraints, int numConstraints,
                 int route[], int used[], int routeCount,
                 Permutation *best, int *hasBest, int forcedStart,
                 long long *validRouteCount) {
    if (routeCount == numAirports) {
        evaluateRoute(airports, constraints, numConstraints, route, routeCount,
                      best, hasBest, forcedStart, validRouteCount);

        return;
    }

    for (int i = 0; i < numAirports; i++) {
        if (used[i]) continue;
        if (forcedStart != -1 && routeCount == 0 && i != forcedStart) continue;

        route[routeCount] = i;
        used[i] = 1;
        searchRoutes(airports, numAirports, constraints, numConstraints,
                     route, used, routeCount + 1, best, hasBest, forcedStart,
                     validRouteCount);
        used[i] = 0;
    }
}

int main() {
    FILE *fin = fopen("D:/C Projects/FlightPlans/FlightPlans/input.txt", "r");
    if (!fin) {
        perror("fopen failed");
        return 1;
    }

    int numAirports;
    if (fscanf(fin, "%d", &numAirports) != 1 ||
        numAirports < 1 || numAirports > MAX_AIRPORTS) {
        fprintf(stderr, "Invalid airport count.\n");
        fclose(fin);
        return 1;
    }

    Airport airports[MAX_AIRPORTS];
    for (int i = 0; i < numAirports; i++) {
        if (fscanf(fin, "%4s %lf %lf", airports[i].code, &airports[i].lat, &airports[i].lon) != 3) {
            fprintf(stderr, "Invalid airport row.\n");
            fclose(fin);
            return 1;
        }
    }

    int numConstraints;
    if (fscanf(fin, "%d", &numConstraints) != 1 ||
        numConstraints < 0 || numConstraints > MAX_CONSTRAINTS) {
        fprintf(stderr, "Invalid constraint count.\n");
        fclose(fin);
        return 1;
    }

    Constraint constraints[MAX_CONSTRAINTS];
    for (int i = 0; i < numConstraints; i++) {
        char beg[5], end[5];
        if (fscanf(fin, "%4s %4s", beg, end) != 2 ||
            findAirportIndex(airports, numAirports, beg) == -1 ||
            findAirportIndex(airports, numAirports, end) == -1) {
            fprintf(stderr, "Invalid constraint row.\n");
            fclose(fin);
            return 1;
        }
        strcpy(constraints[i].beg, beg);
        strcpy(constraints[i].end, end);
    }

    int startIdx = -1;
    char start[5];
    if (fscanf(fin, "%4s", start) == 1) {
        startIdx = findAirportIndex(airports, numAirports, start);
        if (startIdx == -1) {
            fprintf(stderr, "Unknown start airport.\n");
            fclose(fin);
            return 1;
        }
    }
    fclose(fin);

    int route[MAX_AIRPORTS];
    int used[MAX_AIRPORTS] = {0};
    Permutation best = {0};
    int hasBest = 0;
    long long validRouteCount = 0;
    best.distance = -1.0;

    LARGE_INTEGER performanceFrequency;
    LARGE_INTEGER computationStart;
    LARGE_INTEGER computationEnd;
    QueryPerformanceFrequency(&performanceFrequency);
    QueryPerformanceCounter(&computationStart);
    if (startIdx != -1) {
        route[0] = startIdx;
        used[startIdx] = 1;
        searchRoutes(airports, numAirports, constraints, numConstraints,
                     route, used, 1, &best, &hasBest, startIdx,
                     &validRouteCount);
    } else {
        searchRoutes(airports, numAirports, constraints, numConstraints,
                     route, used, 0, &best, &hasBest, -1,
                     &validRouteCount);
    }
    QueryPerformanceCounter(&computationEnd);
    double computationTimeUs =
        (double)(computationEnd.QuadPart - computationStart.QuadPart) * 1000000.0 /
        (double)performanceFrequency.QuadPart;

    FILE *fout = fopen("D:/C Projects/FlightPlans/FlightPlans/output.txt", "w");
    if (!fout) {
        perror("fopen output failed");
        return 1;
    }
    if (hasBest) {
        fprintf(fout, "Valid route count: %lld\n", validRouteCount);
        fprintf(fout, "Computation time \xCE\xBCs: %.0f\n", computationTimeUs);
        for (int i = 0; i < best.count; i++) {
            fprintf(fout, "%s ", airports[best.airports[i]].code);
        }
        fprintf(fout, "%.2f\n", best.distance);
    } else {
        fprintf(fout, "Valid route count: 0\n");
        fprintf(fout, "Computation time \xCE\xBCs: %.0f\n", computationTimeUs);
    }
    fclose(fout);

    return 0;
}