#include "scheduler.h"

int gorev_sec_shortest_job(const SimulasyonMotoru* motor) {
    int secilen_idx = -1;
    const Simulasyon* sim = &motor->sim;

    for (int i = 0; i < sim->gorev_sayisi; i++) {
        const Gorev* g = &sim->gorevler[i];
        if (g->durum != DURUM_BEKLIYOR || g->gelis_zamani > sim->guncel_zaman) {
            continue;
        }

        if (secilen_idx == -1) {
            secilen_idx = i;
            continue;
        }

        const Gorev* secilen = &sim->gorevler[secilen_idx];

        // önce toplam çalışma süresi en kısa olana bakılır
        if (g->sure < secilen->sure) {
            secilen_idx = i;
        } else if (g->sure == secilen->sure) {
            // esitlikte erken gelis zamani, o da esitse kucuk ID
            if (g->gelis_zamani < secilen->gelis_zamani) {
                secilen_idx = i;
            } else if (g->gelis_zamani == secilen->gelis_zamani && g->id < secilen->id) {
                secilen_idx = i;
            }
        }
    }

    return secilen_idx;
}

int gorev_sec_priority(const SimulasyonMotoru* motor) {
    int secilen_idx = -1;
    const Simulasyon* sim = &motor->sim;

    for (int i = 0; i < sim->gorev_sayisi; i++) {
        const Gorev* g = &sim->gorevler[i];
        if (g->durum != DURUM_BEKLIYOR || g->gelis_zamani > sim->guncel_zaman) {
            continue;
        }

        if (secilen_idx == -1) {
            secilen_idx = i;
            continue;
        }

        const Gorev* secilen = &sim->gorevler[secilen_idx];

        // 1 en yüksek öncelik olduğu icin küçük olan secilir
        if (g->oncelik < secilen->oncelik) {
            secilen_idx = i;
        } else if (g->oncelik == secilen->oncelik) {
            if (g->gelis_zamani < secilen->gelis_zamani) {
                secilen_idx = i;
            } else if (g->gelis_zamani == secilen->gelis_zamani && g->id < secilen->id) {
                secilen_idx = i;
            }
        }
    }

    return secilen_idx;
}

int gorev_sec_round_robin(SimulasyonMotoru* motor) {
    while (motor->rr_kuyruk_adet > 0) {
        int gorev_id = motor->rr_kuyruk[motor->rr_kuyruk_basi];
        motor->rr_kuyruk_basi = (motor->rr_kuyruk_basi + 1) % MAX_GOREV_SAYISI;
        motor->rr_kuyruk_adet--;

        for (int i = 0; i < motor->sim.gorev_sayisi; i++) {
            if (motor->sim.gorevler[i].id == gorev_id) {
                if (motor->sim.gorevler[i].durum == DURUM_BEKLIYOR) {
                    return i;
                }
                break;
            }
        }
    }

    return -1;
}

// Kendi algoritmamiz: Konum değişim maliyetini sıfırlamak için önce ayni konumdaki isleri seçer,
// konum değiştirmek gerektiğinde ise en çok is yükü birikmiş konumdaki uzun/kritik isleri önceliklindedir (LPT + Konum Afinitesi)
int gorev_sec_ozgun(const SimulasyonMotoru* motor, int teknisyen_idx) {
    const Simulasyon* sim = &motor->sim;
    const Teknisyen* tek = &sim->teknisyenler[teknisyen_idx];
    Konum tek_konum = tek->son_konum;

    // Her konumdaki bekleyen toplam iş süresini ve o konumdaki aktif teknisyen sayısını hesapla
    int konum_is_yuku[6] = {0};
    int konum_aktif_tek[6] = {0};

    for (int i = 0; i < sim->gorev_sayisi; i++) {
        const Gorev* g = &sim->gorevler[i];
        if (g->durum == DURUM_BEKLIYOR && g->gelis_zamani <= sim->guncel_zaman) {
            konum_is_yuku[g->konum] += g->sure;
        }
    }

    for (int i = 0; i < TEKNISYEN_SAYISI; i++) {
        if (i != teknisyen_idx && sim->teknisyenler[i].durum != TEK_DURUM_BOS) {
            Konum k = sim->teknisyenler[i].son_konum;
            if (k >= 1 && k <= 5) {
                konum_aktif_tek[k]++;
            }
        }
    }

    int ayni_konum_secilen = -1;
    int genel_secilen = -1;
    int en_iyi_genel_skor = -999999;

    for (int i = 0; i < sim->gorev_sayisi; i++) {
        const Gorev* g = &sim->gorevler[i];
        if (g->durum != DURUM_BEKLIYOR || g->gelis_zamani > sim->guncel_zaman) {
            continue;
        }

        // 1. Aşama: Teknisyenin bulunduğu konumda bekleyen is varsa geçiş maliyeti 0'dir
        if (tek_konum != KONUM_YOK && g->konum == tek_konum) {
            if (ayni_konum_secilen == -1) {
                ayni_konum_secilen = i;
            } else {
                const Gorev* mevcut = &sim->gorevler[ayni_konum_secilen];
                // Aynı konumda kalan son uzun işlerin sona kalıp makespan'i uzatmasını önlemek icin
                if (g->sure > mevcut->sure ||
                   (g->sure == mevcut->sure && g->gelis_zamani < mevcut->gelis_zamani) ||
                   (g->sure == mevcut->sure && g->gelis_zamani == mevcut->gelis_zamani && g->id < mevcut->id)) {
                    ayni_konum_secilen = i;
                }
            }
        }

        // 2. Asama: Konum degistirilecekse (veya ilk atamaysa) is yuku en yogun ve teknisyeni az olan konumu sec
        int skor = konum_is_yuku[g->konum] * 10 - konum_aktif_tek[g->konum] * 25 + g->sure * 5 - g->oncelik;
        if (genel_secilen == -1 || skor > en_iyi_genel_skor) {
            en_iyi_genel_skor = skor;
            genel_secilen = i;
        }
    }

    if (ayni_konum_secilen != -1) {
        return ayni_konum_secilen;
    }

    return genel_secilen;
}
