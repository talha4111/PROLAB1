#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_GOREV_SAYISI 2000
#define MAX_TUR_UZUNLUK 40
#define TEKNISYEN_SAYISI 4

typedef enum {
    KONUM_YOK = 0,
    KONUM_LAB_A,
    KONUM_LAB_B,
    KONUM_DERSLIK_1,
    KONUM_DERSLIK_2,
    KONUM_DERSLIK_3
} Konum;

typedef enum {
    DURUM_HENUZ_GELMEDI = 0,
    DURUM_BEKLIYOR,
    DURUM_DEVAM_EDIYOR,
    DURUM_TAMAMLANDI
} GorevDurumu;

typedef enum {
    TEK_DURUM_BOS = 0,
    TEK_DURUM_MESGUL,
    TEK_DURUM_GECISTE
} TeknisyenDurumu;

typedef enum {
    ALGO_YOK = 0,
    ALGO_SHORTEST_JOB = 1,
    ALGO_PRIORITY = 2,
    ALGO_ROUND_ROBIN = 3,
    ALGO_OZGUN = 4
} AlgoritmaTuru;

typedef struct {
    int id;
    int gelis_zamani;
    int sure;
    int oncelik;
    char gorev_turu[MAX_TUR_UZUNLUK];
    Konum konum;

    int kalan_sure;
    GorevDurumu durum;
    int guncel_teknisyen_id;
    int son_teknisyen_id;
    int ilk_baslama_zamani;
    int tamamlanma_zamani;
} Gorev;

typedef struct {
    int id;
    TeknisyenDurumu durum;
    int guncel_gorev_id;
    Konum son_konum;
    int gecis_kalan;
    int dilim_kalan;
    int toplam_calisilan_birim;
} Teknisyen;

typedef struct {
    int guncel_zaman;
    AlgoritmaTuru secili_algo;
    int quantum;
    Gorev gorevler[MAX_GOREV_SAYISI];
    int gorev_sayisi;
    Teknisyen teknisyenler[TEKNISYEN_SAYISI];
    bool baslatildi;
} Simulasyon;

static inline const char* konum_str(Konum k) {
    switch (k) {
        case KONUM_LAB_A:     return "LAB_A";
        case KONUM_LAB_B:     return "LAB_B";
        case KONUM_DERSLIK_1: return "DERSLIK_1";
        case KONUM_DERSLIK_2: return "DERSLIK_2";
        case KONUM_DERSLIK_3: return "DERSLIK_3";
        default:              return "-";
    }
}

static inline Konum str_to_konum(const char* s) {
    if (strcmp(s, "LAB_A") == 0) return KONUM_LAB_A;
    if (strcmp(s, "LAB_B") == 0) return KONUM_LAB_B;
    if (strcmp(s, "DERSLIK_1") == 0) return KONUM_DERSLIK_1;
    if (strcmp(s, "DERSLIK_2") == 0) return KONUM_DERSLIK_2;
    if (strcmp(s, "DERSLIK_3") == 0) return KONUM_DERSLIK_3;
    return KONUM_YOK;
}

static inline const char* durum_str(GorevDurumu d) {
    switch (d) {
        case DURUM_HENUZ_GELMEDI: return "Henuz gelmedi";
        case DURUM_BEKLIYOR:      return "Bekliyor";
        case DURUM_DEVAM_EDIYOR:  return "Devam ediyor";
        case DURUM_TAMAMLANDI:    return "Tamamlandi";
        default:                  return "Bilinmiyor";
    }
}

static inline const char* algo_str(AlgoritmaTuru a) {
    switch (a) {
        case ALGO_SHORTEST_JOB: return "Shortest Job";
        case ALGO_PRIORITY:     return "Priority";
        case ALGO_ROUND_ROBIN:  return "Round Robin";
        case ALGO_OZGUN:        return "Kendi Algoritmaniz";
        default:                return "Secilmedi";
    }
}

#endif
