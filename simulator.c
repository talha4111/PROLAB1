#include "simulator.h"
#include "scheduler.h"

static void rr_kuyruk_temizle(SimulasyonMotoru* motor) {
    motor->rr_kuyruk_basi = 0;
    motor->rr_kuyruk_sonu = 0;
    motor->rr_kuyruk_adet = 0;
}

static void rr_kuyruga_ekle(SimulasyonMotoru* motor, int gorev_id) {
    if (motor->rr_kuyruk_adet >= MAX_GOREV_SAYISI) return;
    motor->rr_kuyruk[motor->rr_kuyruk_sonu] = gorev_id;
    motor->rr_kuyruk_sonu = (motor->rr_kuyruk_sonu + 1) % MAX_GOREV_SAYISI;
    motor->rr_kuyruk_adet++;
}

Gorev* motor_gorev_bul_id(SimulasyonMotoru* motor, int id) {
    for (int i = 0; i < motor->sim.gorev_sayisi; i++) {
        if (motor->sim.gorevler[i].id == id) {
            return &motor->sim.gorevler[i];
        }
    }
    return NULL;
}

bool motor_tum_gorevler_bitti_mi(const SimulasyonMotoru* motor) {
    for (int i = 0; i < motor->sim.gorev_sayisi; i++) {
        if (motor->sim.gorevler[i].durum != DURUM_TAMAMLANDI) {
            return false;
        }
    }
    return true;
}

// boşta olan teknisyenlere T1..T4 sırasıyla görev atar
static void bos_teknisyenlere_atama_yap(SimulasyonMotoru* motor) {
    Simulasyon* sim = &motor->sim;

    for (int i = 0; i < TEKNISYEN_SAYISI; i++) {
        Teknisyen* tek = &sim->teknisyenler[i];
        if (tek->durum != TEK_DURUM_BOS) {
            continue;
        }

        int secilen_idx = -1;
        switch (sim->secili_algo) {
            case ALGO_SHORTEST_JOB:
                secilen_idx = gorev_sec_shortest_job(motor);
                break;
            case ALGO_PRIORITY:
                secilen_idx = gorev_sec_priority(motor);
                break;
            case ALGO_ROUND_ROBIN:
                secilen_idx = gorev_sec_round_robin(motor);
                break;
            case ALGO_OZGUN:
                secilen_idx = gorev_sec_ozgun(motor, i);
                break;
            default:
                secilen_idx = -1;
                break;
        }

        if (secilen_idx != -1) {
            Gorev* g = &sim->gorevler[secilen_idx];
            g->durum = DURUM_DEVAM_EDIYOR;
            g->guncel_teknisyen_id = tek->id;
            tek->guncel_gorev_id = g->id;

            // ilk görevse veya ayni konumsa geçiş suresi yok, farklı konumsa 1 birim geçiş
            if (tek->son_konum == KONUM_YOK || tek->son_konum == g->konum) {
                tek->durum = TEK_DURUM_MESGUL;
                tek->gecis_kalan = 0;
                tek->son_konum = g->konum;
                if (g->ilk_baslama_zamani == -1) {
                    g->ilk_baslama_zamani = sim->guncel_zaman;
                }
            } else {
                tek->durum = TEK_DURUM_GECISTE;
                tek->gecis_kalan = 1;
                tek->son_konum = g->konum;
            }

            if (sim->secili_algo == ALGO_ROUND_ROBIN) {
                tek->dilim_kalan = sim->quantum;
            }
        }
    }
}

void motor_sifirla(SimulasyonMotoru* motor) {
    Simulasyon* sim = &motor->sim;
    sim->guncel_zaman = 0;
    sim->gorev_sayisi = motor->toplam_gorev_sayisi;

    for (int i = 0; i < motor->toplam_gorev_sayisi; i++) {
        sim->gorevler[i] = motor->orijinal_gorevler[i];
        sim->gorevler[i].kalan_sure = sim->gorevler[i].sure;
        sim->gorevler[i].guncel_teknisyen_id = -1;
        sim->gorevler[i].son_teknisyen_id = -1;
        sim->gorevler[i].ilk_baslama_zamani = -1;
        sim->gorevler[i].tamamlanma_zamani = -1;

        if (sim->gorevler[i].gelis_zamani == 0) {
            sim->gorevler[i].durum = DURUM_BEKLIYOR;
        } else {
            sim->gorevler[i].durum = DURUM_HENUZ_GELMEDI;
        }
    }

    for (int i = 0; i < TEKNISYEN_SAYISI; i++) {
        sim->teknisyenler[i].id = i + 1;
        sim->teknisyenler[i].durum = TEK_DURUM_BOS;
        sim->teknisyenler[i].guncel_gorev_id = -1;
        sim->teknisyenler[i].son_konum = KONUM_YOK;
        sim->teknisyenler[i].gecis_kalan = 0;
        sim->teknisyenler[i].dilim_kalan = 0;
        sim->teknisyenler[i].toplam_calisilan_birim = 0;
    }

    rr_kuyruk_temizle(motor);
    if (sim->secili_algo == ALGO_ROUND_ROBIN) {
        int t0_idleri[MAX_GOREV_SAYISI];
        int t0_sayisi = 0;
        for (int i = 0; i < sim->gorev_sayisi; i++) {
            if (sim->gorevler[i].gelis_zamani == 0) {
                t0_idleri[t0_sayisi++] = sim->gorevler[i].id;
            }
        }
        for (int i = 0; i < t0_sayisi - 1; i++) {
            for (int j = i + 1; j < t0_sayisi; j++) {
                if (t0_idleri[i] > t0_idleri[j]) {
                    int tmp = t0_idleri[i];
                    t0_idleri[i] = t0_idleri[j];
                    t0_idleri[j] = tmp;
                }
            }
        }
        for (int i = 0; i < t0_sayisi; i++) {
            rr_kuyruga_ekle(motor, t0_idleri[i]);
        }
    }

    if (sim->secili_algo != ALGO_YOK) {
        bos_teknisyenlere_atama_yap(motor);
    }
}

void motor_baslat(SimulasyonMotoru* motor, const Gorev* gorevler, int gorev_sayisi) {
    motor->toplam_gorev_sayisi = gorev_sayisi;
    for (int i = 0; i < gorev_sayisi; i++) {
        motor->orijinal_gorevler[i] = gorevler[i];
    }
    motor->sim.secili_algo = ALGO_YOK;
    motor->sim.quantum = 1;
    motor->sim.baslatildi = true;
    motor_sifirla(motor);
}

void motor_algoritma_sec(SimulasyonMotoru* motor, AlgoritmaTuru algo, int quantum) {
    motor->sim.secili_algo = algo;
    motor->sim.quantum = (quantum > 0) ? quantum : 1;
    motor_sifirla(motor);
}

void motor_adim_ilerlet(SimulasyonMotoru* motor) {
    Simulasyon* sim = &motor->sim;

    if (sim->secili_algo == ALGO_YOK) {
        return;
    }

    // 1. çalışan görevlerin kalan suresini azalt, geçiştekileri ilerlet
    for (int i = 0; i < TEKNISYEN_SAYISI; i++) {
        Teknisyen* tek = &sim->teknisyenler[i];

        if (tek->durum == TEK_DURUM_GECISTE) {
            tek->gecis_kalan--;
            if (tek->gecis_kalan == 0) {
                tek->durum = TEK_DURUM_MESGUL;
                Gorev* g = motor_gorev_bul_id(motor, tek->guncel_gorev_id);
                if (g && g->ilk_baslama_zamani == -1) {
                    g->ilk_baslama_zamani = sim->guncel_zaman + 1;
                }
            }
        } else if (tek->durum == TEK_DURUM_MESGUL) {
            tek->toplam_calisilan_birim++;
            Gorev* g = motor_gorev_bul_id(motor, tek->guncel_gorev_id);
            if (g) {
                g->kalan_sure--;
                if (sim->secili_algo == ALGO_ROUND_ROBIN) {
                    tek->dilim_kalan--;
                }
            }
        }
    }

    // 2. saati 1 birim ilerlet
    sim->guncel_zaman++;

    // 3. bitenleri tamamla, RR'da süresi dolanları teknisyenden ayır
    int ayrilan_rr_idleri[TEKNISYEN_SAYISI];
    int ayrilan_sayisi = 0;

    for (int i = 0; i < TEKNISYEN_SAYISI; i++) {
        Teknisyen* tek = &sim->teknisyenler[i];
        if (tek->durum == TEK_DURUM_MESGUL && tek->guncel_gorev_id != -1) {
            Gorev* g = motor_gorev_bul_id(motor, tek->guncel_gorev_id);
            if (g) {
                if (g->kalan_sure == 0) {
                    g->durum = DURUM_TAMAMLANDI;
                    g->tamamlanma_zamani = sim->guncel_zaman;
                    g->son_teknisyen_id = tek->id;
                    g->guncel_teknisyen_id = -1;
                    tek->guncel_gorev_id = -1;
                    tek->durum = TEK_DURUM_BOS;
                } else if (sim->secili_algo == ALGO_ROUND_ROBIN && tek->dilim_kalan == 0) {
                    g->durum = DURUM_BEKLIYOR;
                    g->guncel_teknisyen_id = -1;
                    ayrilan_rr_idleri[ayrilan_sayisi++] = g->id;
                    tek->guncel_gorev_id = -1;
                    tek->durum = TEK_DURUM_BOS;
                }
            }
        }
    }

    // 4. yeni gelenleri ve quantum dolanlari sırasıyla kuyruğa ekle
    int yeni_gelen_idleri[MAX_GOREV_SAYISI];
    int yeni_gelen_sayisi = 0;

    for (int i = 0; i < sim->gorev_sayisi; i++) {
        if (sim->gorevler[i].gelis_zamani == sim->guncel_zaman) {
            sim->gorevler[i].durum = DURUM_BEKLIYOR;
            yeni_gelen_idleri[yeni_gelen_sayisi++] = sim->gorevler[i].id;
        }
    }

    if (sim->secili_algo == ALGO_ROUND_ROBIN) {
        for (int i = 0; i < yeni_gelen_sayisi - 1; i++) {
            for (int j = i + 1; j < yeni_gelen_sayisi; j++) {
                if (yeni_gelen_idleri[i] > yeni_gelen_idleri[j]) {
                    int tmp = yeni_gelen_idleri[i];
                    yeni_gelen_idleri[i] = yeni_gelen_idleri[j];
                    yeni_gelen_idleri[j] = tmp;
                }
            }
        }
        for (int i = 0; i < yeni_gelen_sayisi; i++) {
            rr_kuyruga_ekle(motor, yeni_gelen_idleri[i]);
        }
        for (int i = 0; i < ayrilan_sayisi; i++) {
            rr_kuyruga_ekle(motor, ayrilan_rr_idleri[i]);
        }
    }

    // 5. boşta kalan teknisyenlere yeni atamaları yap
    bos_teknisyenlere_atama_yap(motor);
}

void motor_n_adim_ilerlet(SimulasyonMotoru* motor, int n) {
    if (n <= 0) return;
    for (int i = 0; i < n; i++) {
        motor_adim_ilerlet(motor);
    }
}

void motor_zamana_git(SimulasyonMotoru* motor, int hedef_zaman) {
    if (hedef_zaman < 0) return;
    if (hedef_zaman == motor->sim.guncel_zaman) return;

    if (hedef_zaman > motor->sim.guncel_zaman) {
        int fark = hedef_zaman - motor->sim.guncel_zaman;
        motor_n_adim_ilerlet(motor, fark);
    } else {
        // gecmis bir zamana gidilirse sifirdan o zamana kadar tekrar hesapla
        AlgoritmaTuru algo = motor->sim.secili_algo;
        int q = motor->sim.quantum;
        motor_algoritma_sec(motor, algo, q);
        motor_n_adim_ilerlet(motor, hedef_zaman);
    }
}
