#ifndef UI_H
#define UI_H

#include "simulator.h"

void ui_menu_calistir(SimulasyonMotoru* motor);
void ui_algoritma_secim_ekrani(SimulasyonMotoru* motor);
void ui_tum_gorevleri_listele(const SimulasyonMotoru* motor);
void ui_duruma_gore_listele(const SimulasyonMotoru* motor);
void ui_gorev_sorgula(SimulasyonMotoru* motor);
void ui_teknisyen_durumu_goster(const SimulasyonMotoru* motor);

#endif
