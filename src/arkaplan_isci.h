// Bu dosya arka planda çalışacak işçi sınıflarının tanımlarını içerir
// Uzun süren işlemler (fatura oluşturma, rapor üretme, bakım kontrolü) burada tanımlanmış
// Her biri QThread ile ayrı bir izlekte (thread) çalışıyor
// Böylece arayüz donmadan işlemler arka planda devam edebiliyor

#pragma once

#include <QObject>
#include <QString>
#include <mutex>
#include <atomic>
#include "varliklar.h"
#include "depo.h"

// FaturaOlusturucu: Tüm aktif sözleşmeler için toplu fatura metni üreten sınıf
// Sözleşmeleri teker teker gezip her biri için fatura çıktısı hazırlıyor
// İlerleme durumunu sinyal ile bildiriyor ve istenirse iptal edilebiliyor
class FaturaOlusturucu : public QObject {
    Q_OBJECT

public:
    // Kurucu: çalışmak için gereken tüm depoları ve mutex'i parametre olarak alıyor
    FaturaOlusturucu(const Depo<int, KiralamaSozlesmesi>& sozlesmeler,
                     const Depo<std::string, Arac>& araclar,
                     const Depo<std::string, Musteri>& musteriler,
                     std::mutex& veriMutex,
                     QObject* parent = nullptr);

    // İptal bayrağını aktif eder, atomic olduğu için thread-safe
    void iptalEt();

public slots:
    // Arka plan izleğinde çalışacak ana fonksiyon
    // Thread başladığında otomatik olarak bu çağrılır
    void calistir();

signals:
    // İlerleme yüzdesini bildirir (0-100 arası)
    void ilerlemeGuncellendi(int yuzde);

    // İş başarıyla bittiğinde sonucu metin olarak döndürür
    void tamamlandi(const QString& sonuc);

    // Bir şeyler ters giderse hata mesajını bildirir
    void hataOlustu(const QString& mesaj);

private:
    const Depo<int, KiralamaSozlesmesi>& m_sozlesmeler;
    const Depo<std::string, Arac>& m_araclar;
    const Depo<std::string, Musteri>& m_musteriler;
    std::mutex& m_mutex;               // Paylaşılan veriyi koruyan kilit
    std::atomic<bool> m_iptal{false};  // İptal bayrağı
};

// RaporUretici: Kiralama geçmişinden istatistiksel rapor üreten sınıf
// 4 aşamalı rapor üretiyor: genel istatistikler, en çok kiralanan araçlar,
// müşteri bazlı harcamalar ve araç doluluk durumu
class RaporUretici : public QObject {
    Q_OBJECT

public:
    // Kurucu: raporlama için gereken tüm depoları alır
    RaporUretici(const Depo<int, KiralamaSozlesmesi>& sozlesmeler,
                 const Depo<std::string, Arac>& araclar,
                 const Depo<std::string, Musteri>& musteriler,
                 std::mutex& veriMutex,
                 QObject* parent = nullptr);

    // İptal bayrağını aktif eder
    void iptalEt();

public slots:
    // Arka plan izleğinde çalışacak ana fonksiyon
    void calistir();

signals:
    void ilerlemeGuncellendi(int yuzde);
    void tamamlandi(const QString& sonuc);
    void hataOlustu(const QString& mesaj);

private:
    const Depo<int, KiralamaSozlesmesi>& m_sozlesmeler;
    const Depo<std::string, Arac>& m_araclar;
    const Depo<std::string, Musteri>& m_musteriler;
    std::mutex& m_mutex;
    std::atomic<bool> m_iptal{false};
};

// BakimKontrolcu: Tüm araçları tarayarak bakım gereksinimi olan araçları bulan sınıf
// 3+ kez kiralanmış ya da 3+ yaşındaki araçlara bakım önerisi veriyor
class BakimKontrolcu : public QObject {
    Q_OBJECT

public:
    // Kurucu: araç ve sözleşme depolarını alır, kiralama sayılarını hesaplamak için
    BakimKontrolcu(const Depo<std::string, Arac>& araclar,
                   const Depo<int, KiralamaSozlesmesi>& sozlesmeler,
                   std::mutex& veriMutex,
                   QObject* parent = nullptr);

    // İptal bayrağını aktif eder
    void iptalEt();

public slots:
    // Arka plan izleğinde çalışacak ana fonksiyon
    void calistir();

signals:
    void ilerlemeGuncellendi(int yuzde);
    void tamamlandi(const QString& sonuc);
    void hataOlustu(const QString& mesaj);

private:
    const Depo<std::string, Arac>& m_araclar;
    const Depo<int, KiralamaSozlesmesi>& m_sozlesmeler;
    std::mutex& m_mutex;
    std::atomic<bool> m_iptal{false};
};
