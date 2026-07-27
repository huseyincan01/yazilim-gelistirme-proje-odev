// Bu dosya yeni araç ekleme penceresinin (dialog) tanımını içerir
// Kullanıcı bu pencereden plaka, marka, model, yıl, yakıt tipi ve günlük ücret girer
// Girilen veriler doğrulanır, eğer uygunsa ana pencereye geri gönderilir

#pragma once
#include <QDialog>

class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QDialogButtonBox;

// AracDialog: Kullanıcıdan araç bilgilerini almak için açılan pencere
// QDialog'dan türüyor, yani modal bir pencere olarak çalışıyor
class AracDialog : public QDialog {
    Q_OBJECT

public:
    // Pencereyi oluşturur ve içindeki tüm kutucukları hazırlar
    explicit AracDialog(QWidget *parent = nullptr);
    ~AracDialog() = default;

    // Bu fonksiyonlar kullanıcının girdiği bilgileri dışarıya aktarır
    // Ana pencere bu fonksiyonları çağırarak girilen verilere ulaşır
    QString getPlaka() const;
    QString getMarka() const;
    QString getModel() const;
    int getYil() const;
    QString getYakitTipi() const;
    double getGunlukUcret() const;

private slots:
    // Kullanıcı "Kaydet" dediğinde bu fonksiyon çalışır
    // Boş alan var mı, plaka formatı doğru mu diye kontrol eder
    // Her şey tamamsa pencereyi kapatır, yoksa uyarı verir
    void dogrulaVeKapat();

private:
    // Penceredeki giriş kutucuklarının pointer'ları
    QLineEdit *plakaEdit;
    QLineEdit *markaEdit;
    QLineEdit *modelEdit;
    QSpinBox *yilSpin;
    QComboBox *yakitCombo;
    QDoubleSpinBox *ucretSpin;
    QDialogButtonBox *butonKutusu;


};