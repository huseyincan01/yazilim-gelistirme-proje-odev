// Bu dosya ARKIS projesinin temel veri yapılarını barındırıyor
// Araç, Müşteri ve Kiralama Sözleşmesi gibi ana nesnelerin tanımları burada
// Projenin her yerinden kullanılan ortak yapılar bu dosyada toplandı

#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

// Bir aracın o an hangi durumda olduğunu tutan enum
// Müsait, Kirada ya da Bakımda olmak üzere üç durum var
enum class AracDurum {
    Musait,        // Araç kiralanabilir durumda
    Kirada,        // Araç şu anda kiralanmış
    Bakimda        // Araç bakımda, kiralanmaz
};

// Araç durumunu insanların okuyabileceği bir yazıya çeviren yardımcı fonksiyon
// Mesela AracDurum::Musait gelince "Musait" stringi döndürür
inline std::string durumYazisi(AracDurum durum) {
    switch (durum) {
        case AracDurum::Musait:  return "Musait";
        case AracDurum::Kirada:  return "Kirada";
        case AracDurum::Bakimda: return "Bakimda";
    }
    return "Bilinmiyor";
}

// Sistemdeki bir aracı temsil eden yapı
// Plaka, marka, model, yıl, yakıt tipi, günlük ücret ve durum bilgilerini tutar
struct Arac {
    std::string plaka;
    std::string marka;
    std::string model;
    int yil{};
    std::string yakit_tipi;
    double gunluk_ucret{};
    AracDurum durum{AracDurum::Musait};
};

// Aracı cout ile yazdırmak istediğimizde güzel formatlı çıktı almamızı sağlar
// Mesela: [34ABC123] Toyota Corolla (2023) [Yakit: Benzin, Ucret: 850 TL/gun, Durum: Musait]
inline std::ostream& operator<<(std::ostream& os, const Arac& a) {
    os << "[" << a.plaka << "] " << a.marka << " " << a.model
       << " (" << a.yil << ")"
       << " [Yakit: " << a.yakit_tipi
       << ", Ucret: " << a.gunluk_ucret << " TL/gun"
       << ", Durum: " << durumYazisi(a.durum) << "]";
    return os;
}

// Sistemdeki bir müşteriyi temsil eden yapı
// TC kimlik no, isim, soyisim, telefon ve ehliyet bilgilerini tutar
struct Musteri {
    std::string tc_no;
    std::string isim;
    std::string soyisim;
    std::string telefon;
    std::string ehliyet_no;
};

// Müşteriyi cout ile yazdırmak istediğimizde güzel formatlı çıktı verir
inline std::ostream& operator<<(std::ostream& os, const Musteri& m) {
    os << "[TC: " << m.tc_no << "] " << m.isim << " " << m.soyisim
       << " (Tel: " << m.telefon
       << ", Ehliyet: " << m.ehliyet_no << ")";
    return os;
}

// Bir kiralama sözleşmesini temsil eden yapı
// Hangi araç, hangi müşteriye, ne zaman kiralandı, ne zaman biter, toplam ne tutar gibi bilgileri tutar
// Eğer bitis_tarihi boşsa (nullopt) kiralama hala devam ediyor demektir
struct KiralamaSozlesmesi {
    int sozlesme_id{};
    std::string plaka;
    std::string tc_no;
    std::string baslangic_tarihi;                // "YYYY-MM-DD"
    std::optional<std::string> bitis_tarihi;     // Devam ediyorsa nullopt
    double toplam_tutar{};
};

// Sözleşmeyi cout ile yazdırmak istediğimizde güzel formatlı çıktı verir
// Bitiş tarihi yoksa "---" yazar
inline std::ostream& operator<<(std::ostream& os, const KiralamaSozlesmesi& ks) {
    os << "[Sozlesme #" << ks.sozlesme_id << "] Plaka: " << ks.plaka
       << ", TC: " << ks.tc_no
       << ", Baslangic: " << ks.baslangic_tarihi
       << ", Bitis: " << ks.bitis_tarihi.value_or("---")
       << ", Tutar: " << ks.toplam_tutar << " TL";
    return os;
}