#include "parser.h"
#include "simulator.h"
#include "ui.h"

// gorevler.txt bulunamazsa tarayicinin indirdigi gorevler (1).txt vb. dosyalari kontrol eder
static void varsayilan_dosya_bul(char* hedef_yol, size_t boyut) {
    FILE* fp = fopen("gorevler.txt", "r");
    if (fp) {
        fclose(fp);
        strncpy(hedef_yol, "gorevler.txt", boyut - 1);
        return;
    }

    for (int i = 20; i >= 1; i--) {
        char aday[64];
        snprintf(aday, sizeof(aday), "gorevler (%d).txt", i);
        fp = fopen(aday, "r");
        if (fp) {
            fclose(fp);
            strncpy(hedef_yol, aday, boyut - 1);
            return;
        }
    }

    strncpy(hedef_yol, "gorevler.txt", boyut - 1);
}

int main(void) {
    char dosya_yolu[260];
    Gorev gorevler[MAX_GOREV_SAYISI];
    int okunan_sayi = -1;

    while (okunan_sayi <= 0) {
        printf("Gorev dosyasinin yolunu giriniz (Varsayilan: gorevler.txt): ");
        if (!fgets(dosya_yolu, sizeof(dosya_yolu), stdin)) {
            return 1;
        }

        dosya_yolu[strcspn(dosya_yolu, "\r\n")] = '\0';

        if (strlen(dosya_yolu) == 0) {
            varsayilan_dosya_bul(dosya_yolu, sizeof(dosya_yolu));
        }

        okunan_sayi = gorevleri_dosyadan_oku(dosya_yolu, gorevler, MAX_GOREV_SAYISI);

        if (okunan_sayi <= 0) {
            printf("Gecerli gorev yuklenemedi, dosya yolunu kontrol ediniz.\n\n");
        }
    }

    printf("%d gecerli gorev yuklendi (%s).\n", okunan_sayi, dosya_yolu);

    SimulasyonMotoru motor;
    motor_baslat(&motor, gorevler, okunan_sayi);
    ui_menu_calistir(&motor);

    return 0;
}
