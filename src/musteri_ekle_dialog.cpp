// Bu dosya MusteriDialog sınıfının implementasyonunu içerir
// Yeni müşteri ekleme penceresinin nasıl çalıştığı burada kodlanmış
// TC kimlik no formatı ve boş alan kontrolleri yapılıyor

#include "musteri_ekle_dialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QRegularExpression>

// Pencereyi oluşturan kurucu fonksiyon
// TC, isim, soyisim, telefon ve ehliyet alanlarını hazırlıyor
MusteriDialog::MusteriDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Yeni Musteri Ekle");
    resize(300, 200);

    // Form düzeni: sol tarafta etiketler, sağ tarafta giriş alanları
    QFormLayout *layout = new QFormLayout(this);

    // TC kimlik no alanı, en fazla 11 karakter girilebilir
    tcNoEdit = new QLineEdit(this);
    tcNoEdit->setPlaceholderText("11 haneli TC No");
    tcNoEdit->setMaxLength(11);

    isimEdit = new QLineEdit(this);
    soyisimEdit = new QLineEdit(this);

    // Telefon alanı, en fazla 11 karakter
    telefonEdit = new QLineEdit(this);
    telefonEdit->setPlaceholderText("Orn: 05551234567");
    telefonEdit->setMaxLength(11);

    ehliyetNoEdit = new QLineEdit(this);

    // Alanları forma ekliyoruz
    layout->addRow("TC Kimlik No:", tcNoEdit);
    layout->addRow("Isim:", isimEdit);
    layout->addRow("Soyisim:", soyisimEdit);
    layout->addRow("Telefon:", telefonEdit);
    layout->addRow("Ehliyet No:", ehliyetNoEdit);

    // Kaydet ve İptal butonları
    butonKutusu = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addRow(butonKutusu);

    // Buton bağlantıları
    connect(butonKutusu, &QDialogButtonBox::accepted, this, &MusteriDialog::dogrulaVeKapat);
    connect(butonKutusu, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// Kaydet butonuna basıldığında çalışan doğrulama fonksiyonu
// Boş alan kontrolü ve TC formatı kontrolü yapıyor
void MusteriDialog::dogrulaVeKapat() {
    // Zorunlu alanlar boş bırakılmışsa uyarı ver
    if (tcNoEdit->text().trimmed().isEmpty() ||
        isimEdit->text().trimmed().isEmpty() ||
        soyisimEdit->text().trimmed().isEmpty() ||
        telefonEdit->text().trimmed().isEmpty()) {

        QMessageBox::warning(this, "Eksik Bilgi", "Lutfen zorunlu alanlari doldurun!");
        return;
    }

    // TC kimlik no tam 11 haneli ve tamamen sayısal olmalı
    QRegularExpression tcRegex("^[0-9]{11}$");
    if (!tcRegex.match(tcNoEdit->text()).hasMatch()) {
        QMessageBox::warning(this, "Hatalı Format", "TC Kimlik No tam 11 haneli sayisal bir deger olmalidir!");
        return;
    }

    // Her şey tamamsa pencereyi başarılı şekilde kapat
    accept();
}

// Girilen verileri dışarıya döndüren getter fonksiyonları
// Her biri trim yaparak baştaki ve sondaki boşlukları temizliyor
QString MusteriDialog::getTcNo() const { return tcNoEdit->text().trimmed(); }
QString MusteriDialog::getIsim() const { return isimEdit->text().trimmed(); }
QString MusteriDialog::getSoyisim() const { return soyisimEdit->text().trimmed(); }
QString MusteriDialog::getTelefon() const { return telefonEdit->text().trimmed(); }
QString MusteriDialog::getEhliyetNo() const { return ehliyetNoEdit->text().trimmed(); }