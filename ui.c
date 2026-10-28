#include "ui.h"
#include "metrics.h"
#include <ctype.h>

#define SAYFA_BOYUTU 10

static void tamponu_temizle(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static bool tamsayi_oku(int* deger) {
    if (scanf("%d", deger) == 1) {
        tamponu_temizle();
        return true;
    }
    tamponu_temizle();
    return false;
}

static void devam_etmek_icin_bekle(void) {
    printf("\nMenuye donmek icin Enter'a basiniz...");
    getchar();
}

void ui_algoritma_secim_ekrani(SimulasyonMotoru* motor) {
    printf("\n--- Algoritma Secimi ---\n");
    printf("1 - Shortest Job\n");
    printf("2 - Priority\n");
    printf("3 - Round Robin\n");
    printf("4 - Kendi Algoritmaniz\n");
    printf("0 - Geri Don\n");
    printf("Seciminiz: ");

    int secim;
    if (!tamsayi_oku(&secim) || secim < 0 || secim > 4) {
        printf("Gecersiz secim!\n");
        return;
    }

    if (secim == 0) return;

    int quantum = 1;
    if (secim == 3) {
        printf("Quantum suresini giriniz: ");
        if (!tamsayi_oku(&quantum) || quantum < 1) {
            printf("Gecersiz deger, quantum 2 olarak ayarlandi.\n");
            quantum = 2;
        }
    }

    motor_algoritma_sec(motor, (AlgoritmaTuru)secim, quantum);
    printf("Algoritma '%s' secildi. Simulasyon zaman 0'dan baslatildi.\n",
           algo_str((AlgoritmaTuru)secim));
}

static void tablo_basligi_yazdir(void) {
    printf("+------+---------+------+---------+----------------------+-------------+----------------+------------+------------+\n");
    printf("|  ID  |  GELIS  | SURE | ONCELIK |      GOREV TURU      |    KONUM    |     DURUM      | TEKNISYEN  | KALAN SURE |\n");
    printf("+------+---------+------+---------+----------------------+-------------+----------------+------------+------------+\n");
}

static void gorev_satiri_yazdir(const Gorev* g) {
    char tek_str[10];
    if (g->durum == DURUM_DEVAM_EDIYOR && g->guncel_teknisyen_id != -1) {
        snprintf(tek_str, sizeof(tek_str), "T%d", g->guncel_teknisyen_id);
    } else {
        snprintf(tek_str, sizeof(tek_str), "-");
    }

    printf("| %4d | %7d | %4d | %7d | %-20.20s | %-11.11s | %-14.14s |    %-7s | %10d |\n",
           g->id,
           g->gelis_zamani,
           g->sure,
           g->oncelik,
           g->gorev_turu,
           konum_str(g->konum),
           durum_str(g->durum),
           tek_str,
           g->kalan_sure);
}

static void gorev_listesini_sayfala(const Gorev* const* gorev_isaretcileri, int adet) {
    if (adet == 0) {
        printf("Listelenecek gorev bulunamadi.\n");
        return;
    }

    int toplam_sayfa = (adet + SAYFA_BOYUTU - 1) / SAYFA_BOYUTU;
    int sayfa = 0;

    while (1) {
        printf("\nGorev Listesi (Sayfa %d / %d - Toplam %d gorev)\n", sayfa + 1, toplam_sayfa, adet);
        tablo_basligi_yazdir();

        int baslangic = sayfa * SAYFA_BOYUTU;
        int bitis = baslangic + SAYFA_BOYUTU;
        if (bitis > adet) bitis = adet;

        for (int i = baslangic; i < bitis; i++) {
            gorev_satiri_yazdir(gorev_isaretcileri[i]);
        }
        printf("+------+---------+------+---------+----------------------+-------------+----------------+------------+------------+\n");

        if (toplam_sayfa == 1) break;

        printf("[S] Sonraki | [O] Onceki | [C] Cikis: ");
        char komut[32];
        if (!fgets(komut, sizeof(komut), stdin)) break;
        char c = (char)toupper((unsigned char)komut[0]);

        if (c == 'S') {
            if (sayfa + 1 < toplam_sayfa) sayfa++;
            else printf("Son sayfadasiniz.\n");
        } else if (c == 'O') {
            if (sayfa > 0) sayfa--;
            else printf("Ilk sayfadasiniz.\n");
        } else if (c == 'C' || c == '\n') {
            break;
        }
    }
}

void ui_tum_gorevleri_listele(const SimulasyonMotoru* motor) {
    const Simulasyon* sim = &motor->sim;
    const Gorev* isaretciler[MAX_GOREV_SAYISI];

    for (int i = 0; i < sim->gorev_sayisi; i++) {
        isaretciler[i] = &sim->gorevler[i];
    }

    for (int i = 0; i < sim->gorev_sayisi - 1; i++) {
        for (int j = i + 1; j < sim->gorev_sayisi; j++) {
            if (isaretciler[i]->id > isaretciler[j]->id) {
                const Gorev* tmp = isaretciler[i];
                isaretciler[i] = isaretciler[j];
                isaretciler[j] = tmp;
            }
        }
    }

    gorev_listesini_sayfala(isaretciler, sim->gorev_sayisi);
}

void ui_duruma_gore_listele(const SimulasyonMotoru* motor) {
    printf("\nDurum seciniz:\n");
    printf("0 - Henuz gelmedi\n");
    printf("1 - Bekliyor\n");
    printf("2 - Devam ediyor\n");
    printf("3 - Tamamlandi\n");
    printf("Seciminiz (0-3): ");

    int secim;
    if (!tamsayi_oku(&secim) || secim < 0 || secim > 3) {
        printf("Gecersiz secim!\n");
        return;
    }

    GorevDurumu hedef_durum = (GorevDurumu)secim;
    const Simulasyon* sim = &motor->sim;
    const Gorev* isaretciler[MAX_GOREV_SAYISI];
    int sayac = 0;

    for (int i = 0; i < sim->gorev_sayisi; i++) {
        if (sim->gorevler[i].durum == hedef_durum) {
            isaretciler[sayac++] = &sim->gorevler[i];
        }
    }

    for (int i = 0; i < sayac - 1; i++) {
        for (int j = i + 1; j < sayac; j++) {
            if (isaretciler[i]->id > isaretciler[j]->id) {
                const Gorev* tmp = isaretciler[i];
                isaretciler[i] = isaretciler[j];
                isaretciler[j] = tmp;
            }
        }
    }

    gorev_listesini_sayfala(isaretciler, sayac);
}

void ui_gorev_sorgula(SimulasyonMotoru* motor) {
    printf("\nGorev ID giriniz: ");
    int id;
    if (!tamsayi_oku(&id)) {
        printf("Gecersiz ID!\n");
        return;
    }

    Gorev* g = motor_gorev_bul_id(motor, id);
    if (!g) {
        printf("ID %d bulunamadi.\n", id);
        return;
    }

    printf("\n--- Gorev Detayi (ID: %d) ---\n", g->id);
    printf("Gorev Turu            : %s\n", g->gorev_turu);
    printf("Konum                 : %s\n", konum_str(g->konum));
    printf("Gelis Zamani          : %d\n", g->gelis_zamani);
    printf("Toplam Sure           : %d\n", g->sure);
    printf("Kalan Sure            : %d\n", g->kalan_sure);
    printf("Oncelik               : %d\n", g->oncelik);
    printf("Durum                 : %s\n", durum_str(g->durum));

    if (g->durum == DURUM_DEVAM_EDIYOR && g->guncel_teknisyen_id != -1) {
        printf("Guncel Teknisyen      : T%d\n", g->guncel_teknisyen_id);
    } else {
        printf("Guncel Teknisyen      : -\n");
    }

    if (g->ilk_baslama_zamani != -1) {
        printf("Ilk Baslama Zamani    : %d\n", g->ilk_baslama_zamani);
    } else {
        printf("Ilk Baslama Zamani    : -\n");
    }

    if (g->durum == DURUM_TAMAMLANDI) {
        printf("Tamamlanma Zamani     : %d\n", g->tamamlanma_zamani);
        printf("Son Bitiren Teknisyen : T%d\n", g->son_teknisyen_id);
    } else {
        printf("Tamamlanma Zamani     : -\n");
        printf("Son Bitiren Teknisyen : -\n");
    }
}

void ui_teknisyen_durumu_goster(const SimulasyonMotoru* motor) {
    const Simulasyon* sim = &motor->sim;

    printf("\n--- Teknisyen Durumlari ---\n");
    printf("+------------+-----------+-------------+----------+------------+--------------+-----------------+\n");
    printf("| TEKNISYEN  |   DURUM   |    KONUM    | GOREV ID | KALAN SURE | DILIM KALAN  | CALISILAN BIRIM |\n");
    printf("+------------+-----------+-------------+----------+------------+--------------+-----------------+\n");

    for (int i = 0; i < TEKNISYEN_SAYISI; i++) {
        const Teknisyen* tek = &sim->teknisyenler[i];
        const char* durum_metni = "Bos";
        if (tek->durum == TEK_DURUM_MESGUL) {
            durum_metni = "Mesgul";
        } else if (tek->durum == TEK_DURUM_GECISTE) {
            durum_metni = "Geciste";
        }

        char gorev_str[16];
        char kalan_str[16];
        char dilim_str[16];

        if (tek->guncel_gorev_id != -1) {
            snprintf(gorev_str, sizeof(gorev_str), "%d", tek->guncel_gorev_id);
            Gorev* g = motor_gorev_bul_id((SimulasyonMotoru*)motor, tek->guncel_gorev_id);
            if (g) {
                snprintf(kalan_str, sizeof(kalan_str), "%d", g->kalan_sure);
            } else {
                snprintf(kalan_str, sizeof(kalan_str), "-");
            }
        } else {
            snprintf(gorev_str, sizeof(gorev_str), "-");
            snprintf(kalan_str, sizeof(kalan_str), "-");
        }

        if (sim->secili_algo == ALGO_ROUND_ROBIN && tek->durum != TEK_DURUM_BOS) {
            snprintf(dilim_str, sizeof(dilim_str), "%d", tek->dilim_kalan);
        } else {
            snprintf(dilim_str, sizeof(dilim_str), "-");
        }

        printf("| T%-9d | %-9s | %-11s | %-8s | %-10s | %-12s | %-15d |\n",
               tek->id,
               durum_metni,
               konum_str(tek->son_konum),
               gorev_str,
               kalan_str,
               dilim_str,
               tek->toplam_calisilan_birim);
    }
    printf("+------------+-----------+-------------+----------+------------+--------------+-----------------+\n");
}

void ui_menu_calistir(SimulasyonMotoru* motor) {
    while (1) {
        const Simulasyon* sim = &motor->sim;

        printf("\n------------------------------------------------------\n");
        printf("Guncel Zaman: %d | Algoritma: %s", sim->guncel_zaman, algo_str(sim->secili_algo));
        if (sim->secili_algo == ALGO_ROUND_ROBIN) {
            printf(" (Quantum: %d)", sim->quantum);
        }
        printf("\n------------------------------------------------------\n");
        printf(" 1 - Algoritma sec veya degistir\n");
        printf(" 2 - Tum gorevleri listele\n");
        printf(" 3 - Gorevleri durumuna gore listele\n");
        printf(" 4 - Gorev ID ile sorgula\n");
        printf(" 5 - Teknisyenlerin durumunu goster\n");
        printf(" 6 - Zamani 1 birim ilerlet\n");
        printf(" 7 - Zamani N birim ilerlet\n");
        printf(" 8 - Belirli bir zamana git\n");
        printf(" 9 - Istatistikleri goster\n");
        printf("10 - Simulasyonu sifirla\n");
        printf("11 - Cikis\n");
        printf("Seciminiz: ");

        int secim;
        if (!tamsayi_oku(&secim)) {
            printf("Gecersiz giris!\n");
            continue;
        }

        switch (secim) {
            case 1:
                ui_algoritma_secim_ekrani(motor);
                devam_etmek_icin_bekle();
                break;
            case 2:
                ui_tum_gorevleri_listele(motor);
                devam_etmek_icin_bekle();
                break;
            case 3:
                ui_duruma_gore_listele(motor);
                devam_etmek_icin_bekle();
                break;
            case 4:
                ui_gorev_sorgula(motor);
                devam_etmek_icin_bekle();
                break;
            case 5:
                ui_teknisyen_durumu_goster(motor);
                devam_etmek_icin_bekle();
                break;
            case 6:
                if (sim->secili_algo == ALGO_YOK) {
                    printf("Lutfen once algoritma seciniz.\n");
                } else {
                    motor_adim_ilerlet(motor);
                    printf("Zaman 1 birim ilerletildi. Guncel zaman: %d\n", motor->sim.guncel_zaman);
                }
                devam_etmek_icin_bekle();
                break;
            case 7: {
                if (sim->secili_algo == ALGO_YOK) {
                    printf("Lutfen once algoritma seciniz.\n");
                } else {
                    printf("Ilerletilecek birim sayisi (N): ");
                    int n;
                    if (tamsayi_oku(&n) && n > 0) {
                        motor_n_adim_ilerlet(motor, n);
                        printf("Zaman %d birim ilerletildi. Guncel zaman: %d\n", n, motor->sim.guncel_zaman);
                    } else {
                        printf("Gecersiz deger! Pozitif tam sayi giriniz.\n");
                    }
                }
                devam_etmek_icin_bekle();
                break;
            }
            case 8: {
                if (sim->secili_algo == ALGO_YOK) {
                    printf("Lutfen once algoritma seciniz.\n");
                } else {
                    printf("Hedef zaman: ");
                    int hedef;
                    if (tamsayi_oku(&hedef) && hedef >= 0) {
                        motor_zamana_git(motor, hedef);
                        printf("Zaman %d anina gidildi.\n", motor->sim.guncel_zaman);
                    } else {
                        printf("Gecersiz hedef zaman!\n");
                    }
                }
                devam_etmek_icin_bekle();
                break;
            }
            case 9: {
                Istatistikler ist = istatistikleri_hesapla(motor);
                istatistikleri_yazdir(&ist);
                devam_etmek_icin_bekle();
                break;
            }
            case 10:
                motor_sifirla(motor);
                printf("Simulasyon zaman 0'a sifirlandi.\n");
                devam_etmek_icin_bekle();
                break;
            case 11:
                printf("Cikis yapiliyor...\n");
                return;
            default:
                printf("Lutfen 1-11 arasinda bir secenek giriniz.\n");
                devam_etmek_icin_bekle();
                break;
        }
    }
}
