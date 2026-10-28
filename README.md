<div align="center">
  <h1>⚙️ Multi-Technician Task Scheduling Simulator (C99)</h1>
  <p><strong>Konum Değişim Maliyeti (Context/Location Switch Penalty) Kısıtı Altında Çoklu Teknisyen Görev Zamanlama ve Deterministik Simülasyon Motoru</strong></p>
  
  [![Language](https://img.shields.io/badge/Language-C99-00599C?style=for-the-badge&logo=c&logoColor=white)]()
  [![Compiler](https://img.shields.io/badge/Compiler-GCC_%2F_MinGW-A42E2B?style=for-the-badge&logo=gnu&logoColor=white)]()
  [![Domain](https://img.shields.io/badge/Domain-OS_%26_Scheduling_Algorithms-2EA44F?style=for-the-badge)]()
</div>

<br>

## 📖 Proje Hakkında
Bu proje, **Kocaeli Üniversitesi Bilgisayar Mühendisliği Bölümü Programlama Laboratuvarı I (BLM209)** kapsamında geliştirilmiş, modüler **C (C99)** mimarisine sahip deterministik bir **Çoklu Kaynak Görev Zamanlama (Multi-Resource Task Scheduling) Simülatörüdür**.

Klasik işletim sistemi CPU zamanlama algoritmalarından farklı olarak sistem, **5 fiziksel konum** (`LAB_A`, `LAB_B`, `DERSLIK_1`, `DERSLIK_2`, `DERSLIK_3`) arasında görev yapan **4 paralel teknisyenin (`T1–T4`)** oda değiştirdiğinde maruz kaldığı **1 zaman birimlik fiziksel geçiş maliyetini (Location/Context Switch Penalty)** gerçekçi bir durum makinesi (State Machine) ile modeller.

---

## ✨ Temel Mühendislik Özellikleri

### 1. 🧠 4 Farklı Zamanlama Algoritması (`scheduler.c`)
1. **Shortest Job First (SJF — Kesintisiz):** Hazır kuyruktaki en kısa süreli (`sure`) görevi seçer; eşitlik durumunda varış zamanı (`varis_zamani`) ve görev kimliği (`id`) ile deterministik tie-breaking uygular.
2. **Priority Scheduling (Öncelik Tabanlı — Kesintisiz):** Kritikliği en yüksek (`oncelik = 1` en yüksek, `5` en düşük) görevi önceliklendirir.
3. **Round Robin (RR — Kesintili / Zaman Dilimli):**
   - Dairesel FIFO kuyruğu (`rr_kuyruk[MAX_GOREV]`) ve kullanıcı tanımlı **Quantum ($q$)** süresi ile çalışır.
   - Yeni gelen görevlerin, aynı anda süresi dolan görevlerden önce kuyruğa alınmasını sağlayan **kesin olay sıralaması (Event Ordering)** içerir.
   - Teknisyen konum değiştirirken (`INTIKAL` durumu) görev kuantum sayacını dondurarak adil yürütme sağlar.
4. **Özgün Algoritma — Konum Duyarlı İş Yükü Dengeleyici (`gorev_sec_ozgun`):**
   - Konum değişim cezasını (switch overhead) ve kuyruk sonu darboğazlarını (tail bottleneck) minimize etmek için **2 aşamalı hibrit bir sezgisel (heuristic)** yaklaşım kullanır:
     - **Aşama 1 (Sıfır İntikal & LPT — Longest Processing Time):** Teknisyenin bulunduğu odada bekleyen görev varsa, 1 birimlik intikal cezasını sıfırlamak ve uzun görevleri erken eritmek için aynı konumdaki en uzun süreli görevi seçer (`puan = sure * 10 - oncelik`).
     - **Aşama 2 (Konum Bazlı İş Yükü Yoğunluğu & Çarpışma Önleme):** Mevcut odada görev yoksa, diğer odaların toplam bekleyen iş yükünü (`konum_is_yuku`) ve o odalardaki aktif teknisyen sayısını (`konumdaki_teknisyen_sayisi`) analiz ederek boşta kalan teknisyenleri iş yükü en ağır ve teknisyeni en az olan odaya yönlendirir:
       $$\text{Skor} = (\text{Konum İş Yükü} \times 10) - (\text{Odadaki Teknisyen Sayısı} \times 25) + (\text{Görev Süresi} \times 5) - \text{Öncelik}$$

---

### 📊 Deneysel Performans Karşılaştırması (100 Görevlik Benchmark)
Standart 100 görevlik veri seti (`gorevler.txt`) üzerinde 4 teknisyen ile yapılan simülasyon sonuçları:

| Zamanlama Algoritması | Toplam Tamamlanma Süresi (Makespan) | Ortalama Bekleme Süresi | Ortalama Dönüş Süresi (Turnaround) | Genel Teknisyen Verimliliği (Utilization) |
| :--- | :---: | :---: | :---: | :---: |
| **Shortest Job First (SJF)** | 200 birim | **67.35 birim** | **74.82 birim** | %93.38 |
| **Priority Scheduling** | 199 birim | 77.79 birim | 85.26 birim | %93.84 |
| **Round Robin ($q = 5$)** | 205 birim | 112.16 birim | 119.63 birim | %90.98 |
| **Özgün Algoritma (`gorev_sec_ozgun`)** | **170 birim (-%15.0)** | 78.65 birim | 86.12 birim | **%96.76** |

> **Sonuç:** Geliştirilen özgün algoritma, klasik algoritmalarda 199–205 birim bandında sıkışan toplam tamamlanma süresini (makespan) **170 birime düşürerek %15.0 hızlanma** ve **%96.76 kaynak kullanım verimliliği** sağlamıştır.

---

### 2. ⏱️ Deterministik Zaman Yolculuğu (Time-Travel State Replay — `simulator.c`)
- Simülasyon **1 birim ileri**, **$N$ birim ileri** veya doğrudan **herhangi bir $t$ zamanına (geçmiş veya gelecek)** sıçrama yapabilir.
- Geçmiş bir $t$ anına dönüldüğünde motor, `orijinal_gorevler` yedeğinden sistemi $t=0$ durumuna sıfırlayıp hedef $t$ anına kadar deterministik olarak yeniden oynatır (`zamana_git`).

### 3. 🛡️ Hataya Dayanıklı Satır Bazlı Ayrıştırıcı (`parser.c`) ve Konsol Arayüzü (`ui.c`)
- Girdi dosyasındaki eksik sütunları, negatif değerleri, geçersiz konum adlarını ve **mükerrer görev ID'lerini (Duplicate ID)** satır numarasıyla tespit ederek reddeder ve geçerli görevlerle simülasyonu güvenle başlatır.
- Sayfalandırılmış (paginated) ASCII tabloları ile anlık teknisyen durumlarını (`BOSTA`, `INTIKAL`, `CALISIYOR`), aktif görev ilerleme çubuklarını ve performans metriklerini görselleştirir.

---

## 🚀 Derleme ve Çalıştırma

### 1. Repoyu Klonlayın
```bash
git clone https://github.com/talha4111/PROLAB1.git
cd PROLAB1
```

### 2. GCC ile Derleyin
```bash
gcc -std=c99 -Wall -Wextra -O2 main.c parser.c scheduler.c simulator.c metrics.c ui.c -o prolab1
```

### 3. Simülatörü Başlatın
```bash
./prolab1 gorevler.txt
```

---

## 📂 Modüler Mimari Yapı
```text
PROLAB1/
├── main.c         # Giriş noktası, argüman yönetimi ve ana etkileşimli simülasyon döngüsü
├── types.h        # Gorev, Teknisyen, SimDurum, Konum ve AlgoritmaTipi veri yapıları
├── parser.c/.h    # Satır bazlı dosya okuyucu, şema doğrulama ve mükerrer ID denetimi
├── scheduler.c/.h # SJF, Priority, Round Robin (dairesel kuyruk) ve Özgün Algoritma
├── simulator.c/.h # Zaman adımı yürütücüsü, intikal (switch cost) yönetimi ve zamana_git()
├── metrics.c/.h   # Makespan, bekleme/dönüş süreleri ve teknisyen verimlilik hesaplamaları
├── ui.c/.h        # Sayfalandırılmış ASCII tablo arayüzü ve durum görselleştirme katmanı
└── gorevler.txt   # 100 görevlik örnek simülasyon veri seti
```

---

## 👨‍💻 Geliştirici
**Talha Gürler** — Kocaeli Üniversitesi Bilgisayar Mühendisliği Bölümü — [GitHub (@talha4111)](https://github.com/talha4111)
