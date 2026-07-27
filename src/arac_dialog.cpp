// Bu dosya AracDialog sınıfının implementasyonunu içerir
// Yeni araç ekleme penceresinin nasıl çalıştığı, hangi kutucukların olduğu,
// form doğrulamasının nasıl yapıldığı burada kodlanmış

#include "arac_dialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QRegularExpression>

// Pencereyi oluşturan kurucu fonksiyon
// Tüm giriş alanlarını oluşturup form düzenine yerleştiriyor
AracDialog::AracDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Yeni Arac Ekle");
    resize(300, 250);

    // Form düzenleyici: her satırda bir etiket ve bir kutucuk olacak şekilde hizalıyor
    QFormLayout *layout = new QFormLayout(this);

    // Plaka giriş alanı, placeholder ile kullanıcıya örnek gösteriyoruz
    plakaEdit = new QLineEdit(this);
    plakaEdit->setPlaceholderText("Orn: 61 TS 1967");

    markaEdit = new QLineEdit(this);
    modelEdit = new QLineEdit(this);

    // Yıl seçici: 1990 ile 2026 arasında seçim yapılabiliyor
    yilSpin = new QSpinBox(this);
    yilSpin->setRange(1990, 2026);
    yilSpin->setValue(2020);

    // Yakıt tipi açılır listesi
    yakitCombo = new QComboBox(this);
    yakitCombo->addItems({"Benzin", "Dizel", "Elektrik", "Hibrit"});

    // Günlük ücret girişi, sonunda TL yazısı gösteriyor
    ucretSpin = new QDoubleSpinBox(this);
    ucretSpin->setRange(100.0, 50000.0);
    ucretSpin->setSuffix(" TL");

    // Oluşturduğumuz kutucukları forma ekliyoruz
    layout->addRow("Plaka:", plakaEdit);
    layout->addRow("Marka:", markaEdit);
    layout->addRow("Model:", modelEdit);
    layout->addRow("Yil:", yilSpin);
    layout->addRow("Yakit Tipi:", yakitCombo);
    layout->addRow("Gunluk Ucret:", ucretSpin);

    // Kaydet ve İptal butonları
    butonKutusu = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addRow(butonKutusu);

    // Kaydet butonuna basılınca doğrulama yap, iptal basılınca pencereyi kapat
    connect(butonKutusu, &QDialogButtonBox::accepted, this, &AracDialog::dogrulaVeKapat);
    connect(butonKutusu, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// Kullanıcı kaydet dediğinde çalışan doğrulama fonksiyonu
// Önce boş alan kontrolü yapıyor, sonra plaka formatını regex ile kontrol ediyor
// Her şey uygunsa accept() ile pencereyi başarılı şekilde kapatıyor
void AracDialog::dogrulaVeKapat() {
    // Boş alan kontrolü: plaka, marka ve model boş bırakılamaz
    if (plakaEdit->text().trimmed().isEmpty() || markaEdit->text().trimmed().isEmpty() || modelEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Hata", "Lutfen zorunlu alanlari bos birakmayin!");
        return;
    }

    // Plaka formatı kontrolü: 2 rakam + 1-3 harf + 2-4 rakam olmalı
    QRegularExpression plakaRegex("^[0-9]{2}[A-Z]{1,3}[0-9]{2,4}$");
    if (!plakaRegex.match(plakaEdit->text().toUpper()).hasMatch()) {
        QMessageBox::warning(this, "Hata", "Gecersiz plaka formati! Ornek: 61 TS 1967 (Bosluklara dikkat edin)");
        return;
    }

    // Kontroller geçildiyse pencereyi başarılı sinyaliyle kapat
    accept();
}

// Girilen verileri dışarıya döndüren getter fonksiyonları
// Plaka büyük harfe çevrilip boşlukları temizleniyor
QString AracDialog::getPlaka() const { return plakaEdit->text().toUpper().trimmed(); }
QString AracDialog::getMarka() const { return markaEdit->text().trimmed(); }
QString AracDialog::getModel() const { return modelEdit->text().trimmed(); }
int AracDialog::getYil() const { return yilSpin->value(); }
QString AracDialog::getYakitTipi() const { return yakitCombo->currentText(); }
double AracDialog::getGunlukUcret() const { return ucretSpin->value(); }