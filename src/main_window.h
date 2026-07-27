// Bu dosya uygulamanın ana penceresinin tanımını içerir
// Menüler, araç çubukları, sekmeler, gösterge paneli ve arka plan işlemleri
// gibi tüm ana arayüz bileşenlerini yöneten sınıf burada tanımlanmış
// Ayrıca JSON dışa/içe aktarım ve QSettings ile pencere durumu koruma da burada

#pragma once
#include <QMainWindow>
#include <QCloseEvent>
#include <mutex>
#include "depo.h"
#include "varliklar.h"

class QTabWidget;
class QMenu;
class QToolBar;
class QAction;
class QLabel;
class QPushButton;

class AracModel;
class MusteriModel;
class KiralamaModel;
class QSortFilterProxyModel;

class QPieSeries;
class QChart;

class QThread;
class QProgressBar;
class QTextEdit;
class FaturaOlusturucu;
class RaporUretici;
class BakimKontrolcu;

// MainWindow: Uygulamanın ana penceresi
// Her şey bu sınıfın içinden yönetiliyor: sekmeler, menüler, grafikler ve thread'ler
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // Kurucu: tüm arayüz bileşenlerini oluşturup yerleştiriyor
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Pencere kapatılırken çağrılır
    // Pencerenin pozisyonunu ve boyutunu QSettings ile kaydeder
    // Böylece uygulama tekrar açıldığında aynı yerde ve boyutta başlar
    void closeEvent(QCloseEvent *event) override;

private:
    // Arayüz oluşturma fonksiyonları - her biri bir parçayı kuruyor
    void menuleriOlustur();      // Dosya, Düzen, Yardım menülerini oluşturur
    void aracCubuguOlustur();    // Üstteki hızlı erişim butonlarını oluşturur
    void sekmeleriOlustur();     // Araçlar, Müşteriler, Kiralama, Gösterge ve Arka Plan sekmelerini oluşturur
    void durumCubuguOlustur();   // Alt kısımdaki durum çubuğunu oluşturur

    void verileriKaydetJSON();   // Tüm verileri JSON dosyasına dışa aktarır
    void verileriYukleJSON();    // JSON dosyasından verileri içe aktarır

    void grafikGuncelle();       // Gösterge panelindeki pasta grafiğini günceller

    // Arka plan işlemleri sekmesini oluşturan fonksiyon
    void arkaplanSekmesiOlustur(QWidget *sekmesi);

private slots:
    // Arka plan işlemlerini başlatan slotlar
    void faturaOlusturBaslat();    // Toplu fatura oluşturma thread'ini başlatır
    void raporUretBaslat();        // Rapor üretme thread'ini başlatır
    void bakimKontrolBaslat();     // Bakım kontrolü thread'ini başlatır
    void islemIptalEt();           // Çalışan arka plan işlemini iptal eder
    void islemTamamlandi(const QString& sonuc); // İş bitince sonucu gösterir
    void islemHatasi(const QString& mesaj);      // Hata durumunda mesajı gösterir
    void threadTemizle();          // Thread ve işçi nesnelerini temizler

private:
    QTabWidget *sekmePenceresi;  // Ana sekme yöneticisi

    QLabel *durumEtiketi;        // Durum çubuğundaki araç istatistik etiketi

    QPieSeries *aracDurumSerisi; // Pasta grafiğin veri serisi
    QChart *aracDurumGrafik;     // Pasta grafiğin kendisi

    QAction *aracEkleAksiyonu;      // Araç Ekle menü/araç çubuğu komutu
    QAction *musteriEkleAksiyonu;   // Müşteri Ekle menü/araç çubuğu komutu

    // Veri depoları - uygulamanın tüm verileri burada tutuluyor
    Depo<std::string, Arac> aracDeposu;              // Araç verileri
    Depo<std::string, Musteri> musteriDeposu;        // Müşteri verileri
    Depo<int, KiralamaSozlesmesi> sozlesmeDeposu;    // Sözleşme verileri
    
    int sozlesmeIdSayaci = 1; // Yeni sözleşmelere otomatik artan ID vermek için sayaç

    // Qt Model/View nesneleri - tablolarla depoları birbirine bağlıyor
    AracModel *aracModel;          // Araçlar tablosunun modeli
    MusteriModel *musteriModel;    // Müşteriler tablosunun modeli
    KiralamaModel *kiralamaModel;  // Kiralama tablosunun modeli

    // Arka plan thread yönetimi
    std::mutex m_veriMutex;        // Paylaşılan verileri koruyan kilit
    QThread *m_aktifThread = nullptr;         // Şu an çalışan arka plan thread'i
    FaturaOlusturucu *m_faturaIsci = nullptr;  // Fatura işçi nesnesi
    RaporUretici *m_raporIsci = nullptr;       // Rapor işçi nesnesi
    BakimKontrolcu *m_bakimIsci = nullptr;     // Bakım işçi nesnesi
    QProgressBar *m_ilerlemeCubugu = nullptr;  // İlerleme çubuğu
    QTextEdit *m_sonucAlani = nullptr;         // Sonuçların gösterildiği metin alanı
    QPushButton *m_iptalButonu = nullptr;      // İptal butonu
};