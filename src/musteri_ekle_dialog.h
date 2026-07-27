// Bu dosya yeni müşteri ekleme penceresinin tanımını içerir
// Kullanıcı bu pencereden TC, isim, soyisim, telefon ve ehliyet bilgisi girer
// Girilen veriler doğrulanır, uygunsa ana pencereye geri gönderilir

#pragma once
#include <QDialog>

class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QDialogButtonBox;

// MusteriDialog: Kullanıcıdan müşteri bilgilerini almak için açılan pencere
// QDialog'dan türüyor, modal pencere olarak çalışıyor
class MusteriDialog : public QDialog {
    Q_OBJECT

public:
    // Pencereyi oluşturur ve içindeki kutucukları hazırlar
    explicit MusteriDialog(QWidget *parent = nullptr);
    ~MusteriDialog() = default;

    // Girilen bilgileri dışarıya aktaran getter fonksiyonları
    QString getTcNo() const;
    QString getIsim() const;
    QString getSoyisim() const;
    QString getTelefon() const;
    QString getEhliyetNo() const;

private slots:
    // Kaydet butonuna basıldığında çalışan doğrulama fonksiyonu
    // Boş alan ve TC formatı kontrolü yapar
    void dogrulaVeKapat();

private:
    QLineEdit *tcNoEdit;       // TC kimlik no giriş alanı
    QLineEdit *isimEdit;       // İsim giriş alanı
    QLineEdit *soyisimEdit;    // Soyisim giriş alanı
    QLineEdit *telefonEdit;    // Telefon giriş alanı
    QLineEdit *ehliyetNoEdit;  // Ehliyet no giriş alanı
    QDialogButtonBox *butonKutusu; // Kaydet ve İptal butonları
};