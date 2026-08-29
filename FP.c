// Module: FlightPlanOrder
// Purpose: Generate all airport orderings that minimize distance while
//          respecting Beg/End precedence constraints.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    int beg;
    int end;
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

int repeatAirportForCycle(Constraint *constraints, int numConstraints, int numAirports) {
    for (int airport = 0; airport < numAirports; airport++) {
        int hasBeg = 0;
        int hasEnd = 0;
        for (int i = 0; i < numConstraints; i++) {
            if (constraints[i].beg == airport) hasBeg = 1;
            if (constraints[i].end == airport) hasEnd = 1;
        }
        if (hasBeg && hasEnd) return airport;
    }
    return -1;
}

int firstIndexOf(int route[], int len, int value) {
    for (int i = 0; i < len; i++) {
        if (route[i] == value) return i;
    }
    return -1;
}

int lastIndexOf(int route[], int len, int value) {
    for (int i = len - 1; i >= 0; i--) {
        if (route[i] == value) return i;
    }
    return -1;
}

int isValidRoute(int route[], int len, Constraint *constraints, int numConstraints, int forcedStart) {
    if (forcedStart != -1 && len > 0 && route[0] != forcedStart) return 0;

    for (int i = 0; i < numConstraints; i++) {
        int beg = constraints[i].beg;
        int end = constraints[i].end;

        int firstBeg = -1;
        int lastEnd = -1;

        for (int j = 0; j < len; j++) {
            if (route[j] == beg && firstBeg == -1) firstBeg = j;
            if (route[j] == end) lastEnd = j;
        }

        if (firstBeg != -1 && lastEnd != -1 && firstBeg > lastEnd) return 0;
    }

    return 1;
}

void evaluateRoute(Airport *airports, Constraint *constraints, int numConstraints,
                   int route[], int len, Permutation *best, int *hasBest, int forcedStart) {
    if (!isValidRoute(route, len, constraints, numConstraints, forcedStart)) return;

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

int hasCycleAirport(Constraint *constraints, int numConstraints, int airport) {
    int hasBeg = 0;
    int hasEnd = 0;
    for (int i = 0; i < numConstraints; i++) {
        if (constraints[i].beg == airport) hasBeg = 1;
        if (constraints[i].end == airport) hasEnd = 1;
    }
    return hasBeg && hasEnd;
}

void searchRoutes(Airport *airports, int numAirports,
                 Constraint *constraints, int numConstraints,
                 int route[], int used[], int routeCount,
                 Permutation *best, int *hasBest, int forcedStart,
                 int cycleAirports[], int cycleCount) {
    if (routeCount == numAirports) {
        evaluateRoute(airports, constraints, numConstraints, route, routeCount, best, hasBest, forcedStart);

        for (int i = 0; i < cycleCount; i++) {
            int repeatedRoute[MAX_AIRPORTS + 1];
            for (int j = 0; j < routeCount; j++) repeatedRoute[j] = route[j];
            repeatedRoute[routeCount] = cycleAirports[i];
            evaluateRoute(airports, constraints, numConstraints, repeatedRoute, routeCount + 1, best, hasBest, forcedStart);
        }
        return;
    }

    for (int i = 0; i < numAirports; i++) {
        if (used[i]) continue;
        if (forcedStart != -1 && routeCount == 0 && i != forcedStart) continue;

        route[routeCount] = i;
        used[i] = 1;
        searchRoutes(airports, numAirports, constraints, numConstraints,
                     route, used, routeCount + 1, best, hasBest, forcedStart, cycleAirports, cycleCount);
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
    if (fscanf(fin, "%d", &numAirports) != 1) {
        fclose(fin);
        return 1;
    }

    Airport airports[MAX_AIRPORTS];
    for (int i = 0; i < numAirports; i++) {
        if (fscanf(fin, "%s %lf %lf", airports[i].code, &airports[i].lat, &airports[i].lon) != 3) {
            fclose(fin);
            return 1;
        }
    }

    int numConstraints;
    if (fscanf(fin, "%d", &numConstraints) != 1) {
        fclose(fin);
        return 1;
    }

    Constraint constraints[MAX_CONSTRAINTS];
    for (int i = 0; i < numConstraints; i++) {
        char beg[5], end[5];
        if (fscanf(fin, "%s %s", beg, end) != 2) {
            fclose(fin);
            return 1;
        }
        constraints[i].beg = findAirportIndex(airports, numAirports, beg);
        constraints[i].end = findAirportIndex(airports, numAirports, end);
    }

    int startIdx = -1;
    char start[5];
    if (fscanf(fin, "%s", start) == 1) {
        startIdx = findAirportIndex(airports, numAirports, start);
    }
    fclose(fin);

    int cycleAirports[MAX_AIRPORTS];
    int cycleCount = 0;
    for (int airport = 0; airport < numAirports; airport++) {
        if (hasCycleAirport(constraints, numConstraints, airport)) {
            cycleAirports[cycleCount++] = airport;
        }
    }

    int route[MAX_AIRPORTS];
    int used[MAX_AIRPORTS] = {0};
    Permutation best = {0};
    int hasBest = 0;
    best.distance = -1.0;

    if (startIdx != -1) {
        route[0] = startIdx;
        used[startIdx] = 1;
        searchRoutes(airports, numAirports, constraints, numConstraints,
                     route, used, 1, &best, &hasBest, startIdx, cycleAirports, cycleCount);
    } else {
        searchRoutes(airports, numAirports, constraints, numConstraints,
                     route, used, 0, &best, &hasBest, -1, cycleAirports, cycleCount);
    }

    FILE *fout = fopen("D:/C Projects/FlightPlans/FlightPlans/output.txt", "w");
    if (hasBest) {
        fprintf(fout, "1\n");
        for (int i = 0; i < best.count; i++) {
            fprintf(fout, "%s ", airports[best.airports[i]].code);
        }
        fprintf(fout, "%.2f\n", best.distance);
    } else {
        fprintf(fout, "0\n");
    }
    fclose(fout);

    return 0;
}