// Bu dosya yeni kiralama sözleşmesi oluşturma penceresinin tanımını içerir
// Kullanıcı bu pencereden araç seçer, müşteri seçer, tarih girer
// Toplam ücret otomatik olarak hesaplanır ve gösterilir

#pragma once
#include <QDialog>
#include <QDate>

class QComboBox;
class QDateEdit;
class QDialogButtonBox;
class QLabel;

// KiralamaDialog: Yeni kiralama sözleşmesi oluşturmak için açılan pencere
// İçinde araç ve müşteri listeleri, tarih seçiciler ve otomatik fiyat hesaplama var
class KiralamaDialog : public QDialog {
    Q_OBJECT

public:
    // Pencereyi oluşturur
    explicit KiralamaDialog(QWidget *parent = nullptr);
    ~KiralamaDialog() = default;

    // Kullanıcının seçtiği verileri dışarıya döndüren fonksiyonlar
    QString getPlaka() const;
    QString getTcNo() const;
    QString getBaslangicTarihi() const;
    QString getBitisTarihi() const;
    double getToplamTutar() const;

    // Ana pencereden çağrılıp açılır listelere araç ve müşteri ekleyen fonksiyonlar
    void aracEkle(const QString &plakaVeModel, double gunlukUcret);
    void musteriEkle(const QString &isimVeTc);

private slots:
    // Kaydet butonuna basıldığında seçimleri ve tarihleri doğrulayan fonksiyon
    void dogrulaVeKapat();

    // Tarih veya araç değiştiğinde toplam ücreti yeniden hesaplayan fonksiyon
    void tutarHesapla();

private:
    QComboBox *aracCombo;              // Araç seçim listesi
    QComboBox *musteriCombo;           // Müşteri seçim listesi
    QDateEdit *baslangicTarihi;        // Kiralama başlangıç tarihi
    QDateEdit *bitisTarihi;            // Kiralama bitiş tarihi
    QLabel *toplamTutarEtiketi;        // Hesaplanan toplam ücreti gösteren etiket
    QDialogButtonBox *butonKutusu;     // Kaydet ve İptal butonları
};