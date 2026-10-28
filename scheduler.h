#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "simulator.h"

int gorev_sec_shortest_job(const SimulasyonMotoru* motor);
int gorev_sec_priority(const SimulasyonMotoru* motor);
int gorev_sec_round_robin(SimulasyonMotoru* motor);
int gorev_sec_ozgun(const SimulasyonMotoru* motor, int teknisyen_idx);

#endif
