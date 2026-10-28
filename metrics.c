#include "metrics.h"

Istatistikler istatistikleri_hesapla(const SimulasyonMotoru* motor) {
    Istatistikler ist;
    memset(&ist, 0, sizeof(Istatistikler));

    const Simulasyon* sim = &motor->sim;
    ist.toplam_gorev = sim->gorev_sayisi;

    int toplam_bekleme = 0;
    int toplam_sistemde_kalma = 0;
    int tamamlanan_adet = 0;
    int max_bitis = 0;

    for (int i = 0; i < sim->gorev_sayisi; i++) {
        const Gorev* g = &sim->gorevler[i];
        switch (g->durum) {
            case DURUM_HENUZ_GELMEDI:
                ist.durum_henuz_gelmedi++;
                break;
            case DURUM_BEKLIYOR:
                ist.durum_bekliyor++;
                break;
            case DURUM_DEVAM_EDIYOR:
                ist.durum_devam_ediyor++;
                break;
            case DURUM_TAMAMLANDI:
                ist.durum_tamamlandi++;
                tamamlanan_adet++;

                // bekleme = bitis - gelis - toplam calisma suresi
                int bekleme = g->tamamlanma_zamani - g->gelis_zamani - g->sure;
                int sistemde_kalma = g->tamamlanma_zamani - g->gelis_zamani;

                toplam_bekleme += bekleme;
                toplam_sistemde_kalma += sistemde_kalma;

                if (g->tamamlanma_zamani > max_bitis) {
                    max_bitis = g->tamamlanma_zamani;
                }
                break;
        }
    }

    ist.son_bitis_zamani = max_bitis;

    if (tamamlanan_adet > 0) {
        ist.ortalama_hesaplanabilir = true;
        ist.ortalama_bekleme = (double)toplam_bekleme / tamamlanan_adet;
        ist.ortalama_sistemde_kalma = (double)toplam_sistemde_kalma / tamamlanan_adet;
    } else {
        ist.ortalama_hesaplanabilir = false;
    }

    if (sim->guncel_zaman > 0) {
        ist.kullanim_hesaplanabilir = true;
        int toplam_calisilan_hepsi = 0;

        for (int i = 0; i < TEKNISYEN_SAYISI; i++) {
            int calisilan = sim->teknisyenler[i].toplam_calisilan_birim;
            toplam_calisilan_hepsi += calisilan;
            ist.teknisyen_kullanim[i] = ((double)calisilan / sim->guncel_zaman) * 100.0;
        }

        ist.genel_kullanim = ((double)toplam_calisilan_hepsi / (TEKNISYEN_SAYISI * sim->guncel_zaman)) * 100.0;
    } else {
        ist.kullanim_hesaplanabilir = false;
    }

    return ist;
}

void istatistikleri_yazdir(const Istatistikler* ist) {
    printf("\n--- Simulasyon Istatistikleri ---\n");
    printf("Toplam Gorev Sayisi       : %d\n", ist->toplam_gorev);
    printf("  Henuz Gelmedi           : %d\n", ist->durum_henuz_gelmedi);
    printf("  Bekliyor                : %d\n", ist->durum_bekliyor);
    printf("  Devam Ediyor            : %d\n", ist->durum_devam_ediyor);
    printf("  Tamamlandi              : %d\n", ist->durum_tamamlandi);
    printf("---------------------------------\n");

    if (ist->durum_tamamlandi > 0) {
        printf("Son Bitis Zamani          : %d\n", ist->son_bitis_zamani);
    } else {
        printf("Son Bitis Zamani          : -\n");
    }

    if (ist->ortalama_hesaplanabilir) {
        printf("Ortalama Bekleme Suresi   : %.3f\n", ist->ortalama_bekleme);
        printf("Ortalama Sistemde Kalma   : %.3f\n", ist->ortalama_sistemde_kalma);
    } else {
        printf("Ortalama Bekleme Suresi   : hesaplanamaz\n");
        printf("Ortalama Sistemde Kalma   : hesaplanamaz\n");
    }

    printf("---------------------------------\n");
    if (ist->kullanim_hesaplanabilir) {
        for (int i = 0; i < TEKNISYEN_SAYISI; i++) {
            printf("T%d Kullanim Orani         : %%%.3f\n", i + 1, ist->teknisyen_kullanim[i]);
        }
        printf("Genel Kullanim Orani      : %%%.3f\n", ist->genel_kullanim);
    } else {
        for (int i = 0; i < TEKNISYEN_SAYISI; i++) {
            printf("T%d Kullanim Orani         : hesaplanamaz\n", i + 1);
        }
        printf("Genel Kullanim Orani      : hesaplanamaz\n");
    }
}
