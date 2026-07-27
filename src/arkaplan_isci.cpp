// Bu dosya arka plan işçi sınıflarının implementasyonunu içerir
// FaturaOlusturucu, RaporUretici ve BakimKontrolcu sınıflarının
// asıl iş yapan fonksiyonları burada kodlanmış
// Her biri ayrı bir thread'de çalışarak arayüzün donmasını engelliyor

#include "arkaplan_isci.h"
#include <QThread>
#include <map>
#include <numeric>

// FaturaOlusturucu kurucu: gerekli depoları ve mutex'i saklıyor
FaturaOlusturucu::FaturaOlusturucu(
    const Depo<int, KiralamaSozlesmesi>& sozlesmeler,
    const Depo<std::string, Arac>& araclar,
    const Depo<std::string, Musteri>& musteriler,
    std::mutex& veriMutex,
    QObject* parent)
    : QObject(parent)
    , m_sozlesmeler(sozlesmeler)
    , m_araclar(araclar)
    , m_musteriler(musteriler)
    , m_mutex(veriMutex)
{
}

// İptal bayrağını true yapıyor, calistir fonksiyonu her döngüde bunu kontrol ediyor
void FaturaOlusturucu::iptalEt() {
    m_iptal.store(true);
}

// Toplu fatura oluşturma işlemi
// Tüm sözleşmeleri gezip her biri için detaylı fatura metni üretiyor
// Her sözleşmede müşteri ve araç bilgisini de bulup faturaya ekliyor
void FaturaOlusturucu::calistir() {
    QString sonuc;
    int faturaNo = 1;

    try {
        // Paylaşılan verilere erişmeden önce mutex kilidi alıyoruz
        std::lock_guard<std::mutex> kilit(m_mutex);

        const auto& tumSozlesmeler = m_sozlesmeler.tumunu_al();
        int toplam = tumSozlesmeler.size();

        if (toplam == 0) {
            emit hataOlustu("Sistemde hic sozlesme bulunamadi!");
            return;
        }

        sonuc += "========== TOPLU FATURA RAPORU ==========\n\n";
        int sayac = 0;

        for (const auto& [id, ks] : tumSozlesmeler) {
            // Her sözleşmeden önce iptal edilmiş mi diye bakıyoruz
            if (m_iptal.load()) {
                sonuc += "\n[!] Islem kullanici tarafindan iptal edildi.\n";
                emit tamamlandi(sonuc);
                return;
            }

            sonuc += QString("--- Fatura #%1 ---\n").arg(faturaNo++);
            sonuc += QString("Sozlesme ID : %1\n").arg(ks.sozlesme_id);
            sonuc += QString("Plaka       : %1\n").arg(QString::fromStdString(ks.plaka));

            // Müşteri bilgisini depoda arayıp bulursak faturaya ekliyoruz
            auto musteriOpt = m_musteriler.bul(ks.tc_no);
            if (musteriOpt) {
                sonuc += QString("Musteri     : %1 %2 (TC: %3)\n")
                    .arg(QString::fromStdString(musteriOpt->isim))
                    .arg(QString::fromStdString(musteriOpt->soyisim))
                    .arg(QString::fromStdString(musteriOpt->tc_no));
            }

            // Araç bilgisini depoda arayıp bulursak faturaya ekliyoruz
            auto aracOpt = m_araclar.bul(ks.plaka);
            if (aracOpt) {
                sonuc += QString("Arac        : %1 %2 (%3)\n")
                    .arg(QString::fromStdString(aracOpt->marka))
                    .arg(QString::fromStdString(aracOpt->model))
                    .arg(aracOpt->yil);
            }

            sonuc += QString("Tarih       : %1 - %2\n")
                .arg(QString::fromStdString(ks.baslangic_tarihi))
                .arg(ks.bitis_tarihi.has_value()
                    ? QString::fromStdString(ks.bitis_tarihi.value())
                    : "Devam Ediyor");
            sonuc += QString("Toplam Tutar: %1 TL\n\n").arg(ks.toplam_tutar);

            sayac++;
            // İlerleme yüzdesini hesaplayıp arayüze bildiriyoruz
            emit ilerlemeGuncellendi(sayac * 100 / toplam);

            // Gerçek hayatta burada PDF üretimi gibi ağır işler olurdu, şimdilik bekliyoruz
            QThread::msleep(300);
        }

        sonuc += QString("==========================================\n");
        sonuc += QString("Toplam %1 adet fatura olusturuldu.\n").arg(faturaNo - 1);

        emit tamamlandi(sonuc);

    } catch (const std::exception& e) {
        emit hataOlustu(QString("Fatura olusturma hatasi: %1").arg(e.what()));
    }
}

// RaporUretici kurucu: gerekli depoları ve mutex'i saklıyor
RaporUretici::RaporUretici(
    const Depo<int, KiralamaSozlesmesi>& sozlesmeler,
    const Depo<std::string, Arac>& araclar,
    const Depo<std::string, Musteri>& musteriler,
    std::mutex& veriMutex,
    QObject* parent)
    : QObject(parent)
    , m_sozlesmeler(sozlesmeler)
    , m_araclar(araclar)
    , m_musteriler(musteriler)
    , m_mutex(veriMutex)
{
}

void RaporUretici::iptalEt() {
    m_iptal.store(true);
}

// Kiralama geçmişi raporu üretme işlemi
// 4 aşamada çalışır: genel istatistikler, en çok kiralanan araçlar,
// müşteri bazlı harcamalar ve araç doluluk durumu
void RaporUretici::calistir() {
    QString sonuc;

    try {
        std::lock_guard<std::mutex> kilit(m_mutex);

        const auto& tumSozlesmeler = m_sozlesmeler.tumunu_al();
        const auto& tumAraclar = m_araclar.tumunu_al();
        const auto& tumMusteriler = m_musteriler.tumunu_al();

        int toplamAdim = 4;
        int adim = 0;

        sonuc += "========== KIRALAMA GECMISI RAPORU ==========\n\n";

        // Adim 1: Genel istatistikler - toplam araç, müşteri, sözleşme ve gelir
        if (m_iptal.load()) { emit tamamlandi("[!] Iptal edildi."); return; }

        double toplamGelir = 0.0;
        for (const auto& [id, ks] : tumSozlesmeler) {
            toplamGelir += ks.toplam_tutar;
        }

        sonuc += "1. GENEL ISTATISTIKLER\n";
        sonuc += QString("   Toplam Arac Sayisi     : %1\n").arg(tumAraclar.size());
        sonuc += QString("   Toplam Musteri Sayisi  : %1\n").arg(tumMusteriler.size());
        sonuc += QString("   Toplam Sozlesme Sayisi : %1\n").arg(tumSozlesmeler.size());
        sonuc += QString("   Toplam Gelir           : %1 TL\n\n").arg(toplamGelir);

        adim++;
        emit ilerlemeGuncellendi(adim * 100 / toplamAdim);
        QThread::msleep(500);

        // Adim 2: Hangi araç kaç kez kiralanmış, en popüler araçları buluyoruz
        if (m_iptal.load()) { emit tamamlandi("[!] Iptal edildi."); return; }

        std::map<std::string, int> kiralamaSayilari;
        for (const auto& [id, ks] : tumSozlesmeler) {
            kiralamaSayilari[ks.plaka]++;
        }

        sonuc += "2. EN COK KIRALANAN ARACLAR\n";
        for (const auto& [plaka, sayi] : kiralamaSayilari) {
            auto aracOpt = m_araclar.bul(plaka);
            QString aracAdi = aracOpt
                ? QString::fromStdString(aracOpt->marka + " " + aracOpt->model)
                : "Bilinmiyor";
            sonuc += QString("   %1 (%2) - %3 kez kiralanmis\n")
                .arg(QString::fromStdString(plaka))
                .arg(aracAdi)
                .arg(sayi);
        }
        sonuc += "\n";

        adim++;
        emit ilerlemeGuncellendi(adim * 100 / toplamAdim);
        QThread::msleep(500);

        // Adim 3: Her müşteri toplamda ne kadar harcamış
        if (m_iptal.load()) { emit tamamlandi("[!] Iptal edildi."); return; }

        std::map<std::string, double> musteriHarcamalari;
        for (const auto& [id, ks] : tumSozlesmeler) {
            musteriHarcamalari[ks.tc_no] += ks.toplam_tutar;
        }

        sonuc += "3. MUSTERI BAZLI HARCAMALAR\n";
        for (const auto& [tc, harcama] : musteriHarcamalari) {
            auto musteriOpt = m_musteriler.bul(tc);
            QString isim = musteriOpt
                ? QString::fromStdString(musteriOpt->isim + " " + musteriOpt->soyisim)
                : "Bilinmiyor";
            sonuc += QString("   %1 (TC: %2) - Toplam: %3 TL\n")
                .arg(isim)
                .arg(QString::fromStdString(tc))
                .arg(harcama);
        }
        sonuc += "\n";

        adim++;
        emit ilerlemeGuncellendi(adim * 100 / toplamAdim);
        QThread::msleep(500);

        // Adim 4: Araçların durumlarına göre doluluk oranı
        if (m_iptal.load()) { emit tamamlandi("[!] Iptal edildi."); return; }

        int musait = 0, kirada = 0, bakimda = 0;
        for (const auto& [plaka, arac] : tumAraclar) {
            if (arac.durum == AracDurum::Musait) musait++;
            else if (arac.durum == AracDurum::Kirada) kirada++;
            else bakimda++;
        }

        sonuc += "4. ARAC DOLULUK DURUMU\n";
        sonuc += QString("   Musait : %1\n").arg(musait);
        sonuc += QString("   Kirada : %1\n").arg(kirada);
        sonuc += QString("   Bakimda: %1\n\n").arg(bakimda);

        if (tumAraclar.size() > 0) {
            double doluluk = (double)kirada / tumAraclar.size() * 100.0;
            sonuc += QString("   Doluluk Orani: %%1\n").arg(doluluk, 0, 'f', 1);
        }

        adim++;
        emit ilerlemeGuncellendi(adim * 100 / toplamAdim);
        QThread::msleep(300);

        sonuc += "\n=============================================\n";
        sonuc += "Rapor basariyla olusturuldu.\n";

        emit tamamlandi(sonuc);

    } catch (const std::exception& e) {
        emit hataOlustu(QString("Rapor uretme hatasi: %1").arg(e.what()));
    }
}

// BakimKontrolcu kurucu: araç ve sözleşme depolarını alıyor
BakimKontrolcu::BakimKontrolcu(
    const Depo<std::string, Arac>& araclar,
    const Depo<int, KiralamaSozlesmesi>& sozlesmeler,
    std::mutex& veriMutex,
    QObject* parent)
    : QObject(parent)
    , m_araclar(araclar)
    , m_sozlesmeler(sozlesmeler)
    , m_mutex(veriMutex)
{
}

void BakimKontrolcu::iptalEt() {
    m_iptal.store(true);
}

// Araç bakım kontrolü işlemi
// Her aracı tarayarak yaşına ve kiralama sayısına göre bakım önerisi veriyor
// 3+ kez kiralananlar veya 3+ yaşındakiler bakım gerektirir olarak işaretleniyor
void BakimKontrolcu::calistir() {
    QString sonuc;

    try {
        std::lock_guard<std::mutex> kilit(m_mutex);

        const auto& tumAraclar = m_araclar.tumunu_al();
        int toplam = tumAraclar.size();

        if (toplam == 0) {
            emit hataOlustu("Sistemde hic arac bulunamadi!");
            return;
        }

        // Önce her aracın kaç kez kiralandığını hesaplıyoruz
        std::map<std::string, int> kiralamaSayilari;
        for (const auto& [id, ks] : m_sozlesmeler.tumunu_al()) {
            kiralamaSayilari[ks.plaka]++;
        }

        sonuc += "========== BAKIM TAKVIMI KONTROLU ==========\n\n";

        int sayac = 0;
        int bakimGerekli = 0;
        int zatenBakimda = 0;

        for (const auto& [plaka, arac] : tumAraclar) {
            // Her araçtan önce iptal kontrolü
            if (m_iptal.load()) {
                sonuc += "\n[!] Islem kullanici tarafindan iptal edildi.\n";
                emit tamamlandi(sonuc);
                return;
            }

            int kiralamaSayisi = kiralamaSayilari.count(plaka) ? kiralamaSayilari[plaka] : 0;
            int aracYasi = 2026 - arac.yil;

            sonuc += QString("[%1] %2 %3 (%4)\n")
                .arg(QString::fromStdString(plaka))
                .arg(QString::fromStdString(arac.marka))
                .arg(QString::fromStdString(arac.model))
                .arg(arac.yil);

            sonuc += QString("   Durum: %1 | Kiralama Sayisi: %2 | Yas: %3 yil\n")
                .arg(QString::fromStdString(durumYazisi(arac.durum)))
                .arg(kiralamaSayisi)
                .arg(aracYasi);

            // Bakım kararı: zaten bakımdaysa not düş, değilse kriterlere bak
            if (arac.durum == AracDurum::Bakimda) {
                sonuc += "   >> Zaten bakimda.\n\n";
                zatenBakimda++;
            } else if (kiralamaSayisi >= 3 || aracYasi >= 3) {
                sonuc += "   >> [!] BAKIM GEREKLI (";
                if (kiralamaSayisi >= 3) sonuc += QString("cok kiralanmis:%1 ").arg(kiralamaSayisi);
                if (aracYasi >= 3) sonuc += QString("eski arac:%1 yil").arg(aracYasi);
                sonuc += ")\n\n";
                bakimGerekli++;
            } else {
                sonuc += "   >> Durum iyi, bakim gerekmiyor.\n\n";
            }

            sayac++;
            emit ilerlemeGuncellendi(sayac * 100 / toplam);
            QThread::msleep(400);
        }

        sonuc += "=============================================\n";
        sonuc += QString("Taranan arac     : %1\n").arg(toplam);
        sonuc += QString("Bakim gerekli    : %1\n").arg(bakimGerekli);
        sonuc += QString("Zaten bakimda    : %1\n").arg(zatenBakimda);
        sonuc += QString("Durumu iyi       : %1\n").arg(toplam - bakimGerekli - zatenBakimda);

        emit tamamlandi(sonuc);

    } catch (const std::exception& e) {
        emit hataOlustu(QString("Bakim kontrol hatasi: %1").arg(e.what()));
    }
}
