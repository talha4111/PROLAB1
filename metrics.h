#ifndef METRICS_H
#define METRICS_H

#include "simulator.h"

typedef struct {
    int toplam_gorev;
    int durum_henuz_gelmedi;
    int durum_bekliyor;
    int durum_devam_ediyor;
    int durum_tamamlandi;

    bool ortalama_hesaplanabilir;
    double ortalama_bekleme;
    double ortalama_sistemde_kalma;

    bool kullanim_hesaplanabilir;
    double teknisyen_kullanim[TEKNISYEN_SAYISI];
    double genel_kullanim;

    int son_bitis_zamani;
} Istatistikler;

Istatistikler istatistikleri_hesapla(const SimulasyonMotoru* motor);
void istatistikleri_yazdir(const Istatistikler* ist);

#endif
