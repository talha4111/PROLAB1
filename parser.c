#include "parser.h"
#include <ctype.h>

// gelen string sadece rakamlardan mi olusuyor kontrolu
static bool sadece_rakam_mi(const char* str) {
    if (!str || *str == '\0') return false;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}

// boş satır veya # ile baslayan açıkklama satırlarını atlamak için
static bool bos_veya_yorum_mu(const char* satir) {
    while (*satir && isspace((unsigned char)*satir)) {
        satir++;
    }
    return (*satir == '\0' || *satir == '#');
}

int gorevleri_dosyadan_oku(const char* dosya_yolu, Gorev* gorev_dizisi, int max_kapasite) {
    FILE* fp = fopen(dosya_yolu, "r");
    if (!fp) {
        printf("Hata: Dosya acilamadi -> %s\n", dosya_yolu);
        return -1;
    }

    char satir[512];
    int satir_no = 0;
    int gecerli_sayisi = 0;

    while (fgets(satir, sizeof(satir), fp)) {
        satir_no++;
        satir[strcspn(satir, "\r\n")] = '\0';

        if (bos_veya_yorum_mu(satir)) {
            continue;
        }

        if (gecerli_sayisi >= max_kapasite) {
            printf("Uyari (Satir %d): Kapasite siniri (%d) doldu, kalan satirlar alinmadi.\n",
                   satir_no, max_kapasite);
            break;
        }

        char satir_kopya[512];
        strncpy(satir_kopya, satir, sizeof(satir_kopya) - 1);
        satir_kopya[sizeof(satir_kopya) - 1] = '\0';

        char* tokenler[10];
        int token_sayisi = 0;

        char* token = strtok(satir_kopya, " \t");
        while (token != NULL && token_sayisi < 10) {
            tokenler[token_sayisi++] = token;
            token = strtok(NULL, " \t");
        }

        if (token_sayisi != 6) {
            printf("Satir %d reddedildi: Eksik veya fazla alan var (%d alan bulundu).\n",
                   satir_no, token_sayisi);
            continue;
        }

        // 1. ID kontrolü
        if (!sadece_rakam_mi(tokenler[0])) {
            printf("Satir %d reddedildi: Gecersiz ID degeri ('%s').\n", satir_no, tokenler[0]);
            continue;
        }
        int id = atoi(tokenler[0]);
        if (id <= 0) {
            printf("Satir %d reddedildi: ID pozitif olmali (%d).\n", satir_no, id);
            continue;
        }

        // aynı ID daha önce eklendiyse ilkini koru, bunu atla
        bool yinelenen = false;
        for (int i = 0; i < gecerli_sayisi; i++) {
            if (gorev_dizisi[i].id == id) {
                yinelenen = true;
                break;
            }
        }
        if (yinelenen) {
            printf("Satir %d reddedildi: Yinelenen ID (%d).\n", satir_no, id);
            continue;
        }

        // 2. Gelis zamanı kontrolu
        if (!sadece_rakam_mi(tokenler[1])) {
            printf("Satir %d reddedildi: Gelis zamani gecersiz ('%s').\n", satir_no, tokenler[1]);
            continue;
        }
        int gelis_zamani = atoi(tokenler[1]);
        if (gelis_zamani < 0) {
            printf("Satir %d reddedildi: Gelis zamani negatif olamaz (%d).\n", satir_no, gelis_zamani);
            continue;
        }

        // 3. Süre kontrolu
        if (!sadece_rakam_mi(tokenler[2])) {
            printf("Satir %d reddedildi: Calisma suresi gecersiz ('%s').\n", satir_no, tokenler[2]);
            continue;
        }
        int sure = atoi(tokenler[2]);
        if (sure <= 0) {
            printf("Satir %d reddedildi: Sure sifirdan buyuk olmali (%d).\n", satir_no, sure);
            continue;
        }

        // 4. Öncelik kontrolü (1-5 arasi)
        if (!sadece_rakam_mi(tokenler[3])) {
            printf("Satir %d reddedildi: Oncelik tam sayi olmali ('%s').\n", satir_no, tokenler[3]);
            continue;
        }
        int oncelik = atoi(tokenler[3]);
        if (oncelik < 1 || oncelik > 5) {
            printf("Satir %d reddedildi: Oncelik 1-5 arasinda olmali (%d).\n", satir_no, oncelik);
            continue;
        }

        // 5. Görev türü uzunluk kontrolu
        if (strlen(tokenler[4]) == 0 || strlen(tokenler[4]) >= MAX_TUR_UZUNLUK) {
            printf("Satir %d reddedildi: Gorev turu en fazla 39 karakter olabilir.\n", satir_no);
            continue;
        }

        // 6. Konum kontrolü
        Konum konum = str_to_konum(tokenler[5]);
        if (konum == KONUM_YOK) {
            printf("Satir %d reddedildi: Tanimsiz konum ('%s').\n", satir_no, tokenler[5]);
            continue;
        }

        Gorev* g = &gorev_dizisi[gecerli_sayisi];
        g->id = id;
        g->gelis_zamani = gelis_zamani;
        g->sure = sure;
        g->oncelik = oncelik;
        strncpy(g->gorev_turu, tokenler[4], sizeof(g->gorev_turu) - 1);
        g->gorev_turu[sizeof(g->gorev_turu) - 1] = '\0';
        g->konum = konum;

        g->kalan_sure = sure;
        g->durum = DURUM_HENUZ_GELMEDI;
        g->guncel_teknisyen_id = -1;
        g->son_teknisyen_id = -1;
        g->ilk_baslama_zamani = -1;
        g->tamamlanma_zamani = -1;

        gecerli_sayisi++;
    }

    fclose(fp);
    return gecerli_sayisi;
}
