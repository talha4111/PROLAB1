#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "types.h"

typedef struct {
    Simulasyon sim;
    Gorev orijinal_gorevler[MAX_GOREV_SAYISI];
    int toplam_gorev_sayisi;

    int rr_kuyruk[MAX_GOREV_SAYISI];
    int rr_kuyruk_basi;
    int rr_kuyruk_sonu;
    int rr_kuyruk_adet;
} SimulasyonMotoru;

void motor_baslat(SimulasyonMotoru* motor, const Gorev* gorevler, int gorev_sayisi);
void motor_algoritma_sec(SimulasyonMotoru* motor, AlgoritmaTuru algo, int quantum);
void motor_sifirla(SimulasyonMotoru* motor);

void motor_adim_ilerlet(SimulasyonMotoru* motor);
void motor_n_adim_ilerlet(SimulasyonMotoru* motor, int n);
void motor_zamana_git(SimulasyonMotoru* motor, int hedef_zaman);

Gorev* motor_gorev_bul_id(SimulasyonMotoru* motor, int id);
bool motor_tum_gorevler_bitti_mi(const SimulasyonMotoru* motor);

#endif
