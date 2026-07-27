// Bu dosya KiralamaDialog sınıfının implementasyonunu içerir
// Yeni kiralama sözleşmesi penceresinin nasıl çalıştığı burada kodlanmış
// Tarih değiştiğinde fiyat otomatik güncelleniyor, doğrulama yapılıyor

#include "kiralama_sozlesme_dialog.h"
#include <QFormLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>

// Pencereyi oluşturan kurucu fonksiyon
// Açılır listeler, tarih seçiciler ve fiyat göstergesi burada hazırlanıyor
KiralamaDialog::KiralamaDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Yeni Kiralama Sozlesmesi");
    resize(350, 250);

    QFormLayout *layout = new QFormLayout(this);

    // Araç açılır listesi, ilk eleman "Lutfen Seciniz..." olarak ayarlanıyor
    aracCombo = new QComboBox(this);
    aracCombo->addItem("Lutfen Seciniz...", 0.0);

    // Müşteri açılır listesi
    musteriCombo = new QComboBox(this);
    musteriCombo->addItem("Lutfen Seciniz...");

    // Başlangıç tarihi bugüne, bitiş tarihi yarına ayarlanıyor
    baslangicTarihi = new QDateEdit(QDate::currentDate(), this);
    baslangicTarihi->setCalendarPopup(true); // Takvim ikonu çıkar

    bitisTarihi = new QDateEdit(QDate::currentDate().addDays(1), this);
    bitisTarihi->setCalendarPopup(true);

    // Toplam tutar etiketi, kalın ve büyük fontla gösteriliyor
    toplamTutarEtiketi = new QLabel("0 TL", this);
    toplamTutarEtiketi->setStyleSheet("font-weight: bold; font-size: 14px;");

    layout->addRow("Arac:", aracCombo);
    layout->addRow("Musteri:", musteriCombo);
    layout->addRow("Alis Tarihi:", baslangicTarihi);
    layout->addRow("Teslim Tarihi:", bitisTarihi);
    layout->addRow("Toplam Tutar:", toplamTutarEtiketi);

    butonKutusu = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addRow(butonKutusu);

    // Tarih veya araç değiştikçe fiyatı otomatik yeniden hesaplıyoruz
    connect(baslangicTarihi, &QDateEdit::dateChanged, this, &KiralamaDialog::tutarHesapla);
    connect(bitisTarihi, &QDateEdit::dateChanged, this, &KiralamaDialog::tutarHesapla);
    connect(aracCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KiralamaDialog::tutarHesapla);

    // Pencere açıldığında ilk hesaplamayı yapıyoruz
    tutarHesapla();

    // Kaydet ve İptal butonlarının bağlantıları
    connect(butonKutusu, &QDialogButtonBox::accepted, this, &KiralamaDialog::dogrulaVeKapat);
    connect(butonKutusu, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// Tarih veya araç değiştiğinde çağrılan fonksiyon
// İki tarih arasındaki gün farkını hesaplayıp günlük ücretle çarpıyor
void KiralamaDialog::tutarHesapla() {
    int gunFarki = baslangicTarihi->date().daysTo(bitisTarihi->date());

    // Seçili aracın günlük ücretini combo box'ın data kısmından alıyoruz
    double secilenUcret = 0.0;
    if (aracCombo->currentIndex() > 0) {
        secilenUcret = aracCombo->currentData().toDouble();
    }

    if (gunFarki <= 0) {
        toplamTutarEtiketi->setText("Hatali Tarih!");
        toplamTutarEtiketi->setStyleSheet("color: red; font-weight: bold; font-size: 14px;");
    } else {
        double toplam = gunFarki * secilenUcret;
        toplamTutarEtiketi->setText(QString::number(toplam) + " TL");
        toplamTutarEtiketi->setStyleSheet("color: green; font-weight: bold; font-size: 14px;");
    }
}

// Kaydet butonuna basıldığında çalışan doğrulama fonksiyonu
// Araç ve müşteri seçilmiş mi, tarihler mantıklı mı kontrol ediyor
void KiralamaDialog::dogrulaVeKapat() {
    // Açılır listelerden seçim yapılmamışsa uyarı ver
    if (aracCombo->currentIndex() == 0 || musteriCombo->currentIndex() == 0) {
        QMessageBox::warning(this, "Eksik Seçim", "Lutfen listeden bir arac ve musteri seciniz!");
        return;
    }

    // Bitiş tarihi başlangıçtan önce veya aynı gün olamaz
    if (baslangicTarihi->date() >= bitisTarihi->date()) {
        QMessageBox::warning(this, "Hatali Tarih", "Teslim tarihi, alis tarihinden en az 1 gun sonra olmalidir!");
        return;
    }

    accept();
}

// Seçilen aracın plakasını döndürür
// Combo box'taki metinden " - " işaretine kadar olan kısmı alır
QString KiralamaDialog::getPlaka() const {
    QString text = aracCombo->currentText();
    return text.split(" - ").first().trimmed();
}

// Seçilen müşterinin TC numarasını döndürür
// Combo box'taki metinden parantez içindeki kısmı çıkarır
QString KiralamaDialog::getTcNo() const {
    QString text = musteriCombo->currentText();
    int idx1 = text.indexOf('(');
    int idx2 = text.indexOf(')');
    if (idx1 != -1 && idx2 != -1) {
        return text.mid(idx1 + 1, idx2 - idx1 - 1).trimmed();
    }
    return text;
}

// Seçilen başlangıç tarihini yyyy-MM-dd formatında döndürür
QString KiralamaDialog::getBaslangicTarihi() const {
    return baslangicTarihi->date().toString("yyyy-MM-dd");
}

// Seçilen bitiş tarihini yyyy-MM-dd formatında döndürür
QString KiralamaDialog::getBitisTarihi() const {
    return bitisTarihi->date().toString("yyyy-MM-dd");
}

// Hesaplanan toplam ücreti döndürür (gün farkı x günlük ücret)
double KiralamaDialog::getToplamTutar() const {
    int gunFarki = baslangicTarihi->date().daysTo(bitisTarihi->date());
    double secilenUcret = (aracCombo->currentIndex() > 0) ? aracCombo->currentData().toDouble() : 0.0;
    return gunFarki > 0 ? (gunFarki * secilenUcret) : 0.0;
}

// Ana pencereden çağrılarak araç listesine yeni araç ekler
// plakaVeModel gösterim metni, gunlukUcret ise hesaplama için saklanan data
void KiralamaDialog::aracEkle(const QString &plakaVeModel, double gunlukUcret) {
    aracCombo->addItem(plakaVeModel, gunlukUcret);
}

// Ana pencereden çağrılarak müşteri listesine yeni müşteri ekler
void KiralamaDialog::musteriEkle(const QString &isimVeTc) {
    musteriCombo->addItem(isimVeTc);
}