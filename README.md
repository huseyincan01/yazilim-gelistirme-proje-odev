# ARKIS - Araç Kiralama Sistemi

Bu proje, C++ ve Qt framework kullanılarak geliştirilmiş bir araç kiralama otomasyonudur. (Yazılım Geliştirme Vize Ödevi)

## Geliştiren

- **Ad Soyad:** Hüseyin Taşkan
- **Öğrenci No:** 444029

## Proje Özellikleri

- **Araç Yönetimi:** Sisteme yeni araç ekleme, silme ve detaylı araç listesini (plaka, marka, model, yıl, yakıt tipi vb.) görüntüleme.
- **Müşteri Yönetimi:** Müşteri kayıtlarını (TC, isim, telefon, ehliyet no) oluşturma, listeleme ve silme.
- **Kiralama İşlemleri:** Sistemdeki müsait araçları müşterilere kiralama, kiralama sözleşmeleri oluşturma ve araç teslim alma işlemleri.
- **Otomatik Tutar Hesaplama:** Kiralama tarihlerine ve aracın günlük ücretine göre toplam kiralama bedelini otomatik hesaplama.
- **Gösterge Paneli (Dashboard):** Sistemdeki araçların durumunu (müsait, kirada, bakımda) anlık olarak pasta grafiği ile görüntüleme.
- **Veri Kayıt ve Okuma:** Program kapanıp açıldığında verilerin kaybolmaması için ikili (binary) dosyalara okuma/yazma. Ayrıca tüm verileri JSON formatında dışa ve içe aktarabilme.
- **Arka Plan İşlemleri (Multi-threading):** Arayüzün donmasını engellemek için toplu fatura oluşturma, istatistiksel rapor üretme ve araç bakım takvimi kontrolü gibi işlemleri arka planda ayrı thread'lerde yapabilme.

## Kullanılan Teknolojiler

- C++20
- Qt Framework (Qt Widgets, Qt Charts)
- Standart C++ Kütüphaneleri (STL)
- JSON (Qt JSON)
