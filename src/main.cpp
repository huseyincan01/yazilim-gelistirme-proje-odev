// Bu dosya ARKIS (Araç Kiralama Sistemi) projemizin ana giriş noktasıdır
// Konsol ekranındaki demo testleri ve Qt arayüzünün başlatılması buradan yapılıyor

#include "../include/varliklar.h"
#include "../include/depo.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>
#include <set>
#include <string>
#include <fstream>
#include <QApplication>
#include "main_window.h"

// Dosya işlemleri için yardımcı fonksiyonlar
// String verilerini ikili (binary) formata çevirerek diske yazıyoruz
// Önce string'in uzunluğunu, sonra da karakterleri kaydediyoruz
void yazString(std::ofstream& out, const std::string& str){
    size_t len = str.size();
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    out.write(str.data(), len);
}

// Dosyadan ikili formatta yazılmış string'i geri okuyoruz
std::string okuString(std::ifstream& in) {
    size_t len;
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string str(len, '\0');
    in.read(&str[0], len);
    return str;
}

// Araçları dosyaya kaydetme fonksiyonu
// Depodaki tüm araçları tek tek dönüp özelliklerini dosyaya yazıyor
void dosyaya_kaydet(const std::string& dosya_yolu, const Depo<std::string, Arac>& depo) {
    std::ofstream out(dosya_yolu, std::ios::binary);
    size_t adet = depo.boyut();
    out.write(reinterpret_cast<const char*>(&adet), sizeof(adet));

    for (const auto& [plaka, arac] : depo.tumunu_al()) {
        yazString(out, plaka);
        yazString(out, arac.plaka);
        yazString(out, arac.marka);
        yazString(out, arac.model);
        out.write(reinterpret_cast<const char*>(&arac.yil), sizeof(arac.yil));
        yazString(out, arac.yakit_tipi);
        out.write(reinterpret_cast<const char*>(&arac.gunluk_ucret), sizeof(arac.gunluk_ucret));
        out.write(reinterpret_cast<const char*>(&arac.durum), sizeof(arac.durum));
    }
}

// Araçları dosyadan geri okuma fonksiyonu
// Dosyadaki araç sayısı kadar dönüp aracı depoya geri ekliyor
Depo<std::string, Arac> dosyadan_oku(const std::string& dosya_yolu){
    Depo<std::string, Arac> depo;
    std::ifstream in(dosya_yolu, std::ios::binary);
    if (!in) return depo;

    size_t adet;
    in.read(reinterpret_cast<char*>(&adet), sizeof(adet));

    for (size_t i = 0; i < adet; ++i) {
        std::string anahtar = okuString(in);
        Arac arac;
        arac.plaka = okuString(in);
        arac.marka = okuString(in);
        arac.model = okuString(in);
        in.read(reinterpret_cast<char*>(&arac.yil), sizeof(arac.yil));
        arac.yakit_tipi = okuString(in);
        in.read(reinterpret_cast<char*>(&arac.gunluk_ucret), sizeof(arac.gunluk_ucret));
        in.read(reinterpret_cast<char*>(&arac.durum), sizeof(arac.durum));
        depo.ekle(anahtar, arac);
    }
    return depo;
}

// Müşterileri dosyaya kaydetme fonksiyonu
void musteri_dosyaya_kaydet(const std::string& dosya_yolu, const Depo<std::string, Musteri>& depo) {
    std::ofstream out(dosya_yolu, std::ios::binary);
    size_t adet = depo.boyut();
    out.write(reinterpret_cast<const char*>(&adet), sizeof(adet));

    for (const auto& [tc, musteri] : depo.tumunu_al()) {
        yazString(out, tc);
        yazString(out, musteri.tc_no);
        yazString(out, musteri.isim);
        yazString(out, musteri.soyisim);
        yazString(out, musteri.telefon);
        yazString(out, musteri.ehliyet_no);
    }
}

// Müşterileri dosyadan okuma fonksiyonu
Depo<std::string, Musteri> musteri_dosyadan_oku(const std::string& dosya_yolu) {
    Depo<std::string, Musteri> depo;
    std::ifstream in(dosya_yolu, std::ios::binary);
    if (!in) return depo;

    size_t adet;
    in.read(reinterpret_cast<char*>(&adet), sizeof(adet));

    for (size_t i = 0; i < adet; ++i) {
        std::string anahtar = okuString(in);
        Musteri m;
        m.tc_no = okuString(in);
        m.isim = okuString(in);
        m.soyisim = okuString(in);
        m.telefon = okuString(in);
        m.ehliyet_no = okuString(in);
        depo.ekle(anahtar, m);
    }
    return depo;
}

// Kiralama sözleşmelerini dosyaya kaydetme fonksiyonu
void sozlesme_dosyaya_kaydet(const std::string& dosya_yolu, const Depo<int, KiralamaSozlesmesi>& depo) {
    std::ofstream out(dosya_yolu, std::ios::binary);
    size_t adet = depo.boyut();
    out.write(reinterpret_cast<const char*>(&adet), sizeof(adet));

    for (const auto& [id, ks] : depo.tumunu_al()) {
        out.write(reinterpret_cast<const char*>(&id), sizeof(id));
        out.write(reinterpret_cast<const char*>(&ks.sozlesme_id), sizeof(ks.sozlesme_id));
        yazString(out, ks.plaka);
        yazString(out, ks.tc_no);
        yazString(out, ks.baslangic_tarihi);
        bool bitis_var = ks.bitis_tarihi.has_value();
        out.write(reinterpret_cast<const char*>(&bitis_var), sizeof(bitis_var));
        if (bitis_var) {
            yazString(out, ks.bitis_tarihi.value());
        }
        out.write(reinterpret_cast<const char*>(&ks.toplam_tutar), sizeof(ks.toplam_tutar));
    }
}

// Kiralama sözleşmelerini dosyadan okuma fonksiyonu
Depo<int, KiralamaSozlesmesi> sozlesme_dosyadan_oku(const std::string& dosya_yolu) {
    Depo<int, KiralamaSozlesmesi> depo;
    std::ifstream in(dosya_yolu, std::ios::binary);
    if (!in) return depo;

    size_t adet;
    in.read(reinterpret_cast<char*>(&adet), sizeof(adet));

    for (size_t i = 0; i < adet; ++i) {
        int anahtar;
        in.read(reinterpret_cast<char*>(&anahtar), sizeof(anahtar));
        KiralamaSozlesmesi ks;
        in.read(reinterpret_cast<char*>(&ks.sozlesme_id), sizeof(ks.sozlesme_id));
        ks.plaka = okuString(in);
        ks.tc_no = okuString(in);
        ks.baslangic_tarihi = okuString(in);
        bool bitis_var;
        in.read(reinterpret_cast<char*>(&bitis_var), sizeof(bitis_var));
        if (bitis_var) {
            ks.bitis_tarihi = okuString(in);
        } else {
            ks.bitis_tarihi = std::nullopt;
        }
        in.read(reinterpret_cast<char*>(&ks.toplam_tutar), sizeof(ks.toplam_tutar));
        depo.ekle(anahtar, ks);
    }
    return depo;
}


// Programın asıl çalıştığı ana fonksiyon
int main(int argc, char *argv[]){
    std::cout << "=== ARKIS - Arac Kiralama Sistemi ===\n\n";

    // Örnek araçlarımızı depoya ekliyoruz
    Depo<std::string, Arac> araclar;

    araclar.ekle("34ABC123", {
        "34ABC123", "Toyota", "Corolla",
        2023, "Benzin", 850.0, AracDurum::Musait
    });
    araclar.ekle("06DEF456", {
        "06DEF456", "Volkswagen", "Passat",
        2022, "Dizel", 1100.0, AracDurum::Kirada
    });
    araclar.ekle("35GHI789", {
        "35GHI789", "Renault", "Clio",
        2024, "Benzin", 650.0, AracDurum::Musait
    });
    araclar.ekle("16JKL012", {
        "16JKL012", "Ford", "Focus",
        2021, "Dizel", 750.0, AracDurum::Bakimda
    });

    std::cout << "Arac sayisi: " << araclar.boyut() << "\n\n";

    // Tüm araçları konsolda güzelce listeliyoruz
    std::cout << "--- Arac Katalogu ---\n";
    for (const auto& [plaka, arac] : araclar.tumunu_al()) {
        std::cout << "  " << arac << "\n";
    }
    std::cout << "\n";

    // Örnek müşterileri depomuza ekliyoruz
    Depo<std::string, Musteri> musteriler;

    musteriler.ekle("12345678901", {
        "12345678901", "Ali", "Yilmaz", "555-0101", "B-123456"
    });
    musteriler.ekle("98765432109", {
        "98765432109", "Zeynep", "Kara", "555-0102", "B-654321"
    });
    musteriler.ekle("11223344556", {
        "11223344556", "Mehmet", "Demir", "555-0103", "B-112233"
    });

    std::cout << "Musteri sayisi: " << musteriler.boyut() << "\n\n";

    // Örnek kiralama sözleşmelerimizi oluşturuyoruz
    Depo<int, KiralamaSozlesmesi> sozlesmeler;

    sozlesmeler.ekle(1, {1, "06DEF456", "12345678901",
        "2025-04-01", "2025-04-05", 4400.0});
    sozlesmeler.ekle(2, {2, "34ABC123", "98765432109",
        "2025-04-10", std::nullopt, 0.0});
    sozlesmeler.ekle(3, {3, "35GHI789", "11223344556",
        "2025-03-20", "2025-03-25", 3250.0});

    std::cout << "--- Kiralama Sozlesmeleri ---\n";
    for (const auto& [id, sozlesme] : sozlesmeler.tumunu_al()) {
        std::cout << "  " << sozlesme << "\n";
    }

    // Durumu müsait olan araçları buluyoruz
    auto musait_araclar = araclar.filtrele([](const Arac& a) {
       return a.durum == AracDurum::Musait;
    });

    // Bulduğumuz araçların plakalarını tekrarsız tutmak için bir set oluşturduk
    std::set<std::string> benzersiz_plakalar;
    for (const auto& arac : musait_araclar) {
        benzersiz_plakalar.insert(arac.plaka);
    }

    // Sözleşmeleri tarihe göre gruplamak için bir harita kullanıyoruz
    std::map<std::string, std::vector<KiralamaSozlesmesi>> takvim;

    for (const auto& [id, sozlesme] : sozlesmeler.tumunu_al()) {
        takvim[sozlesme.baslangic_tarihi].push_back(sozlesme);
    }

    // Hangi tarihte hangi araçlar kiralanmış onları yazdırıyoruz
    for (const auto& [tarih, sozlesme_vektoru] : takvim) {
        std::cout << "\n" << tarih << " tarihinde kiralanan araba plakalari\n";
        for(const auto& sozlesme : sozlesme_vektoru) {
            std::cout << sozlesme.plaka << ", ";
        }
        std::cout <<"\n";
    }

    // Bugüne kadar ne kadar gelir elde etmişiz onu hesaplıyoruz
    double toplam_gelir = std::accumulate(
          sozlesmeler.tumunu_al().begin()
        , sozlesmeler.tumunu_al().end()
        , 0.0
        ,[](double toplam, const auto& sozlesmem) {
            return toplam+sozlesmem.second.toplam_tutar;
        });

    std::cout << "Toplam gelir: " << toplam_gelir << "\n";

    // Belirli bir müşterinin ne zaman araç kiraladığını buluyoruz
    std::string aranan_tc = "12345678901"; 
    std::cout << "\nMusteri Gecmisi (TC: " << aranan_tc << ")\n";

    auto musteri_gecmisi = sozlesmeler.filtrele([&aranan_tc](const KiralamaSozlesmesi& sozlesme) {
            return sozlesme.tc_no == aranan_tc;
    });

    for (const auto &sozlesme : musteri_gecmisi) {
        std::cout << "\n" << sozlesme << "\n";
    }

    // Hangi aracın daha çok tercih edildiğini hesaplıyoruz
    std::cout << "\n--- En Cok Kiralanan Arac ---\n";

    std::map<std::string, int> kiralama_sayilari;
    for (const auto& [id, sozlesme] : sozlesmeler.tumunu_al()) {
        kiralama_sayilari[sozlesme.plaka]++;
    }

    auto en_cok = std::max_element(
        kiralama_sayilari.begin(), kiralama_sayilari.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });

    if (en_cok != kiralama_sayilari.end()) {
        std::cout << "Plaka: " << en_cok->first
                  << " (" << en_cok->second << " kez kiralanmis)\n";
    }

    // Dosya yazma testini yapıyoruz, veriler diske gidiyor
    std::cout << "\n--- Dosya Islemleri ---\n";
    std::string arac_dosya   = "araclar.bin";
    std::string musteri_dosya = "musteriler.bin";
    std::string sozlesme_dosya = "sozlesmeler.bin";

    dosyaya_kaydet(arac_dosya, araclar);
    musteri_dosyaya_kaydet(musteri_dosya, musteriler);
    sozlesme_dosyaya_kaydet(sozlesme_dosya, sozlesmeler);
    std::cout << "Tum veriler basariyla diske kaydedildi.\n";

    // Şimdi diskten verileri geri alıyoruz
    Depo<std::string, Arac> diskten_araclar = dosyadan_oku(arac_dosya);
    Depo<std::string, Musteri> diskten_musteriler = musteri_dosyadan_oku(musteri_dosya);
    Depo<int, KiralamaSozlesmesi> diskten_sozlesmeler = sozlesme_dosyadan_oku(sozlesme_dosya);

    std::cout << "Diskten okunan arac sayisi:     " << diskten_araclar.boyut() << "\n";
    std::cout << "Diskten okunan musteri sayisi:   " << diskten_musteriler.boyut() << "\n";
    std::cout << "Diskten okunan sozlesme sayisi:  " << diskten_sozlesmeler.boyut() << "\n";

    std::cout << "\n=== Konsol demo tamamlandi, Qt arayuzu baslatiliyor... ===\n";

    // Ve asıl uygulamanın görsel arayüzü buradan itibaren çalışmaya başlıyor
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}