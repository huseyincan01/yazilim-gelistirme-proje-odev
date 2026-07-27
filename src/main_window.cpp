// Bu dosya MainWindow sınıfının implementasyonunu içerir
// Uygulamanın ana penceresinin tüm davranışları burada kodlanmış
// Menüler, sekmeler, tablo işlemleri, JSON import/export, grafik güncelleme
// ve arka plan thread yönetimi gibi her şey bu dosyadan yönetiliyor
// Proje dosyaları arasında en büyük ve en kapsamlı olan bu dosyadır

#include "main_window.h"
#include "arac_dialog.h"
#include "musteri_ekle_dialog.h"
#include "kiralama_sozlesme_dialog.h"
#include "arac_model.h"
#include "musteri_model.h"
#include "kiralama_model.h"
#include "arkaplan_isci.h"
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QTabWidget>
#include <QStatusBar>
#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QThread>
#include <QProgressBar>
#include <QTextEdit>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>

// Ana pencereyi oluşturan kurucu fonksiyon
// Pencere boyutunu ayarlıyor, eski oturumdan kalan pencere pozisyonunu geri yüklüyor
// Modelleri oluşturup tüm arayüz bileşenlerini tek tek kuruyor
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("ARKIS - Arac Kiralama Sistemi");
    resize(900, 600);

    // Önceki oturumdan kalan pencere pozisyon ve boyutunu geri yükle
    QSettings settings("BenimSirket", "Arkis");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    // Model nesnelerini oluştur, her biri kendi deposuyla çalışacak
    aracModel = new AracModel(aracDeposu, this);
    musteriModel = new MusteriModel(musteriDeposu, this);
    kiralamaModel = new KiralamaModel(sozlesmeDeposu, this);

    // Arayüz bileşenlerini sırayla oluştur
    menuleriOlustur();
    aracCubuguOlustur();
    sekmeleriOlustur();
    durumCubuguOlustur();

    grafikGuncelle();
}

// Yıkıcı: uygulama kapanırken çalışan arka plan thread'i varsa düzgünce kapatır
// Önce iptal sinyali gönderir, sonra thread'in bitmesini bekler
MainWindow::~MainWindow() {
    if (m_aktifThread && m_aktifThread->isRunning()) {
        if (m_faturaIsci) m_faturaIsci->iptalEt();
        if (m_raporIsci) m_raporIsci->iptalEt();
        if (m_bakimIsci) m_bakimIsci->iptalEt();
        m_aktifThread->quit();
        m_aktifThread->wait();
    }
}

// Pencere kapatılırken pencere pozisyonu ve boyutunu kaydet
// Bir dahaki açılışta aynı yerde ve boyutta başlasın diye
void MainWindow::closeEvent(QCloseEvent *event) {
    QSettings settings("BenimSirket", "Arkis");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

// Dosya, Düzen ve Yardım menülerini oluşturan fonksiyon
// Ayrıca araç ve müşteri ekleme diyaloglarını da bu menü aksiyonlarına bağlıyor
void MainWindow::menuleriOlustur() {
    QMenu *dosyaMenusu = menuBar()->addMenu("Dosya");
    QAction *kaydetAksiyonu = dosyaMenusu->addAction("Disa Aktar (JSON)");
    QAction *yukleAksiyonu = dosyaMenusu->addAction("Ice Aktar (JSON)");
    dosyaMenusu->addSeparator();

    connect(kaydetAksiyonu, &QAction::triggered, this, &MainWindow::verileriKaydetJSON);
    connect(yukleAksiyonu, &QAction::triggered, this, &MainWindow::verileriYukleJSON);

    QAction *cikisAksiyonu = dosyaMenusu->addAction("Cikis");
    connect(cikisAksiyonu, &QAction::triggered, qApp, &QApplication::quit);

    QMenu *duzenMenusu = menuBar()->addMenu("Duzen");

    musteriEkleAksiyonu = duzenMenusu->addAction("Musteri Ekle");
    aracEkleAksiyonu = duzenMenusu->addAction("Arac Ekle");

    QMenu *yardimMenusu = menuBar()->addMenu("Yardim");
    QAction *hakkindaAksiyonu = yardimMenusu->addAction("Hakkinda");
    connect(hakkindaAksiyonu, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "Hakkinda", "ARKIS - Arac Kiralama Sistemi\nGelisen Teknolojiler Vize Odevi");
    });



    // diyalog bağlantıları

    connect(aracEkleAksiyonu, &QAction::triggered, this, [this]() {
        AracDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            Arac a;
            a.plaka = dialog.getPlaka().toStdString();
            a.marka = dialog.getMarka().toStdString();
            a.model = dialog.getModel().toStdString();
            a.yil = dialog.getYil();
            a.yakit_tipi = dialog.getYakitTipi().toStdString();
            a.gunluk_ucret = dialog.getGunlukUcret();
            a.durum = AracDurum::Musait;
            
            if (aracDeposu.ekle(a.plaka, a)) {
                aracModel->satirEkle(a);
                grafikGuncelle();
                statusBar()->showMessage("Arac basariyla eklendi.", 3000);
            } else {
                statusBar()->showMessage("Hata: Bu plaka zaten kayitli!", 3000);
            }
        }
    });

    connect(musteriEkleAksiyonu, &QAction::triggered, this, [this]() {
        MusteriDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            Musteri m;
            m.tc_no = dialog.getTcNo().toStdString();
            m.isim = dialog.getIsim().toStdString();
            m.soyisim = dialog.getSoyisim().toStdString();
            m.telefon = dialog.getTelefon().toStdString();
            m.ehliyet_no = dialog.getEhliyetNo().toStdString();
            
            if (musteriDeposu.ekle(m.tc_no, m)) {
                musteriModel->satirEkle(m);
                statusBar()->showMessage("Musteri basariyla eklendi.", 3000);
            } else {
                statusBar()->showMessage("Hata: Bu TC No zaten kayitli!", 3000);
            }
        }
    });
}







// Üst kısımdaki hızlı erişim araç çubuğunu oluşturur
// Menüdeki aksiyonları paylaşarak hem menüden hem araç çubuğundan aynı işlemi yapmayı sağlar
void MainWindow::aracCubuguOlustur() {
    QToolBar *aracCubugu = addToolBar("Ana Arac Cubugu");
    aracCubugu->addAction(aracEkleAksiyonu);   // Menüdeki aksiyonu paylaşır
    aracCubugu->addAction(musteriEkleAksiyonu); // Menüdeki aksiyonu paylaşır
    aracCubugu->addSeparator();

    QAction *cikisAksiyonu = aracCubugu->addAction("Cikis");
    connect(cikisAksiyonu, &QAction::triggered, qApp, &QApplication::quit);
}




// Uygulamanın 5 ana sekmesini oluşturan fonksiyon
// 1.Araçlar 2.Müşteriler 3.Kiralama Sözleşmeleri 4.Gösterge Paneli 5.Arka Plan İşlemleri
// Her sekmenin kendi tablosu, arama kutusu ve işlem butonları var
void MainWindow::sekmeleriOlustur() {
    sekmePenceresi = new QTabWidget(this);


    // ========== 1. ARAÇLAR SEKMESİ ==========
    QWidget *araclarSekmesi = new QWidget();
    QVBoxLayout *aracLayout = new QVBoxLayout(araclarSekmesi);

    // Üst kısım: arama ve filtre alanı
    QHBoxLayout *aramaLayout = new QHBoxLayout();
    QLineEdit *aramaKutusu = new QLineEdit();
    aramaKutusu->setPlaceholderText("Plaka veya marka ile ara...");
    QPushButton *araButonu = new QPushButton("Ara");
    QPushButton *temizleButonu = new QPushButton("Temizle");

    aramaLayout->addWidget(aramaKutusu);
    aramaLayout->addWidget(araButonu);
    aramaLayout->addWidget(temizleButonu);
    aracLayout->addLayout(aramaLayout);

    connect(temizleButonu, &QPushButton::clicked, aramaKutusu, &QLineEdit::clear);

    // Orta kısım: araç listesi tablosu
    QTableView *aracTablosu = new QTableView();
    aracTablosu->setSelectionBehavior(QAbstractItemView::SelectRows);
    aracTablosu->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    QSortFilterProxyModel *aracProxy = new QSortFilterProxyModel(this);
    aracProxy->setSourceModel(aracModel);
    aracProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    aracProxy->setFilterKeyColumn(-1);
    aracTablosu->setModel(aracProxy);

    connect(aramaKutusu, &QLineEdit::textChanged, aracProxy, &QSortFilterProxyModel::setFilterFixedString);

    aracLayout->addWidget(aracTablosu);

    // Alt kısım: işlem butonları
    QHBoxLayout *butonLayout = new QHBoxLayout();
    QPushButton *ekleButonu = new QPushButton("Araç Ekle");
    QPushButton *duzenleButonu = new QPushButton("Düzenle");
    QPushButton *silButonu = new QPushButton("Sil");
    QPushButton *yenileButonu = new QPushButton("Yenile");

    connect(ekleButonu, &QPushButton::clicked, aracEkleAksiyonu, &QAction::trigger);
    connect(duzenleButonu, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Bilgi", "Duzenleme islevi henüz aktif degil (Sadece Sil/Ekle).");
    });
    connect(silButonu, &QPushButton::clicked, this, [this, aracTablosu, aracProxy]() {
        auto index = aracTablosu->selectionModel()->currentIndex();
        if (index.isValid()) {
            int row = aracProxy->mapToSource(index).row();
            QString plaka = aracModel->data(aracModel->index(row, 0)).toString();
            if (aracDeposu.sil(plaka.toStdString())) {
                aracModel->satirSil(row);
                grafikGuncelle();
                statusBar()->showMessage("Arac silindi.", 3000);
            }
        } else {
            QMessageBox::warning(this, "Hata", "Lutfen silmek istediginiz araci secin.");
        }
    });
    connect(yenileButonu, &QPushButton::clicked, this, [this]() {
        aracModel->verileriGuncelle(); // Yenile butonu: kullanici istegi ile tam sifirlama
    });

    butonLayout->addWidget(ekleButonu);
    butonLayout->addWidget(duzenleButonu);
    butonLayout->addWidget(silButonu);
    butonLayout->addWidget(yenileButonu);
    butonLayout->addStretch(); // Butonları sola yasla, sağ boşluk kalsın

    aracLayout->addLayout(butonLayout);
    // aracLayout zaten aracSekmesi'nin parent'ı olduğu için setLayout çağırmaya gerek yok



    // ========== 2. MÜŞTERİLER SEKMESİ ==========
    QWidget *musterilerSekmesi = new QWidget();
    QVBoxLayout *musteriLayout = new QVBoxLayout(musterilerSekmesi);

    // Üst: arama
    QHBoxLayout *musteriAramaLayout = new QHBoxLayout();
    QLineEdit *musteriAramaKutusu = new QLineEdit();
    musteriAramaKutusu->setPlaceholderText("İsim, soyisim veya TC ile ara...");
    QPushButton *musteriAraButonu = new QPushButton("Ara");

    musteriAramaLayout->addWidget(musteriAramaKutusu);
    musteriAramaLayout->addWidget(musteriAraButonu);
    musteriLayout->addLayout(musteriAramaLayout);

    // Orta: müşteri tablosu
    QTableView *musteriTablosu = new QTableView();
    musteriTablosu->setSelectionBehavior(QAbstractItemView::SelectRows);
    musteriTablosu->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QSortFilterProxyModel *musteriProxy = new QSortFilterProxyModel(this);
    musteriProxy->setSourceModel(musteriModel);
    musteriProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    musteriProxy->setFilterKeyColumn(-1);
    musteriTablosu->setModel(musteriProxy);

    connect(musteriAramaKutusu, &QLineEdit::textChanged, musteriProxy, &QSortFilterProxyModel::setFilterFixedString);

    musteriLayout->addWidget(musteriTablosu);

    // Alt: butonlar
    QHBoxLayout *musteriButonLayout = new QHBoxLayout();
    QPushButton *mEkleButonu = new QPushButton("Müşteri Ekle");
    QPushButton *mDuzenleButonu = new QPushButton("Düzenle");
    QPushButton *mSilButonu = new QPushButton("Sil");

    connect(mEkleButonu, &QPushButton::clicked, musteriEkleAksiyonu, &QAction::trigger);
    connect(mDuzenleButonu, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Bilgi", "Duzenleme islevi henüz aktif degil (Sadece Sil/Ekle).");
    });
    connect(mSilButonu, &QPushButton::clicked, this, [this, musteriTablosu, musteriProxy]() {
        auto index = musteriTablosu->selectionModel()->currentIndex();
        if (index.isValid()) {
            int row = musteriProxy->mapToSource(index).row();
            QString tc = musteriModel->data(musteriModel->index(row, 0)).toString();
            if (musteriDeposu.sil(tc.toStdString())) {
                musteriModel->satirSil(row);
                statusBar()->showMessage("Musteri silindi.", 3000);
            }
        } else {
            QMessageBox::warning(this, "Hata", "Lutfen silmek istediginiz musteriyi secin.");
        }
    });

    musteriButonLayout->addWidget(mEkleButonu);
    musteriButonLayout->addWidget(mDuzenleButonu);
    musteriButonLayout->addWidget(mSilButonu);
    musteriButonLayout->addStretch();

    musteriLayout->addLayout(musteriButonLayout);



    // ========== 3. KİRALAMA SÖZLEŞMELERİ SEKMESİ ==========
    QWidget *kiralamaSekmesi = new QWidget();
    QVBoxLayout *kiralamaLayout = new QVBoxLayout(kiralamaSekmesi);

    // Form alanı
    QFormLayout *formLayout = new QFormLayout();
    QLineEdit *plakaGirdi = new QLineEdit();
    plakaGirdi->setPlaceholderText("Plaka giriniz...");
    QLineEdit *tcGirdi = new QLineEdit();
    tcGirdi->setPlaceholderText("TC Kimlik No giriniz...");

    formLayout->addRow("Plaka:", plakaGirdi);
    formLayout->addRow("TC Kimlik:", tcGirdi);
    kiralamaLayout->addLayout(formLayout);

    // Sözleşme tablosu
    QTableView *sozlesmeTablosu = new QTableView();
    sozlesmeTablosu->setSelectionBehavior(QAbstractItemView::SelectRows);
    sozlesmeTablosu->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    QSortFilterProxyModel *kiralamaProxy = new QSortFilterProxyModel(this);
    kiralamaProxy->setSourceModel(kiralamaModel);
    kiralamaProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    kiralamaProxy->setFilterKeyColumn(-1);
    sozlesmeTablosu->setModel(kiralamaProxy);
    
    kiralamaLayout->addWidget(sozlesmeTablosu);

    // Alt butonlar
    QHBoxLayout *kiralamaButonLayout = new QHBoxLayout();
    QPushButton *kiralaButonu = new QPushButton("Aracı Kirala");
    QPushButton *teslimAlButonu = new QPushButton("Aracı Teslim Al");
    QPushButton *iptalButonu = new QPushButton("Sözleşmeyi İptal");

    kiralamaButonLayout->addWidget(kiralaButonu);
    kiralamaButonLayout->addWidget(teslimAlButonu);
    kiralamaButonLayout->addWidget(iptalButonu);
    kiralamaButonLayout->addStretch();

    kiralamaLayout->addLayout(kiralamaButonLayout);

    connect(teslimAlButonu, &QPushButton::clicked, this, [this, sozlesmeTablosu, kiralamaProxy]() {
        auto index = sozlesmeTablosu->selectionModel()->currentIndex();
        if (index.isValid()) {
            int row = kiralamaProxy->mapToSource(index).row();
            int sozlesmeId = kiralamaModel->data(kiralamaModel->index(row, 0)).toInt();
            
            auto opt = sozlesmeDeposu.bul(sozlesmeId);
            if (opt) {
                auto aracOpt = aracDeposu.bul(opt.value().plaka);
                if (aracOpt) {
                    Arac a = aracOpt.value();
                    a.durum = AracDurum::Musait;
                    aracDeposu.sil(a.plaka);
                    aracDeposu.ekle(a.plaka, a);
                    int aracSatir = aracModel->plakaIleSatirBul(a.plaka);
                    if (aracSatir >= 0) aracModel->satirGuncelle(aracSatir, a);
                }
                int sozlesmeSatir = kiralamaModel->idIleSatirBul(sozlesmeId);
                sozlesmeDeposu.sil(sozlesmeId);
                if (sozlesmeSatir >= 0) kiralamaModel->satirSil(sozlesmeSatir);
                grafikGuncelle();
                statusBar()->showMessage("Arac teslim alindi ve sozlesme kapatildi.", 3000);
            }
        } else {
            QMessageBox::warning(this, "Hata", "Islem yapmak icin bir sozlesme secin.");
        }
    });

    connect(iptalButonu, &QPushButton::clicked, this, [this, sozlesmeTablosu, kiralamaProxy]() {
        auto index = sozlesmeTablosu->selectionModel()->currentIndex();
        if (index.isValid()) {
            int row = kiralamaProxy->mapToSource(index).row();
            int sozlesmeId = kiralamaModel->data(kiralamaModel->index(row, 0)).toInt();
            
            auto opt = sozlesmeDeposu.bul(sozlesmeId);
            if (opt) {
                auto aracOpt = aracDeposu.bul(opt.value().plaka);
                if (aracOpt) {
                    Arac a = aracOpt.value();
                    a.durum = AracDurum::Musait;
                    aracDeposu.sil(a.plaka);
                    aracDeposu.ekle(a.plaka, a);
                    int aracSatir = aracModel->plakaIleSatirBul(a.plaka);
                    if (aracSatir >= 0) aracModel->satirGuncelle(aracSatir, a);
                }
                int sozlesmeSatir = kiralamaModel->idIleSatirBul(sozlesmeId);
                sozlesmeDeposu.sil(sozlesmeId);
                if (sozlesmeSatir >= 0) kiralamaModel->satirSil(sozlesmeSatir);
                grafikGuncelle();
                statusBar()->showMessage("Sozlesme iptal edildi.", 3000);
            }
        } else {
            QMessageBox::warning(this, "Hata", "Islem yapmak icin bir sozlesme secin.");
        }
    });

    // ---- DİYALOG bağlantısı ----
    connect(kiralaButonu, &QPushButton::clicked, this, [this]() {
        KiralamaDialog dialog(this);

        for (const auto& [plaka, arac] : aracDeposu.tumunu_al()) {
            if (arac.durum == AracDurum::Musait) {
                QString metin = QString::fromStdString(plaka + " - " + arac.marka + " " + arac.model);
                dialog.aracEkle(metin, arac.gunluk_ucret);
            }
        }

        for (const auto& [tc, musteri] : musteriDeposu.tumunu_al()) {
            QString metin = QString::fromStdString(musteri.isim + " " + musteri.soyisim + " (" + tc + ")");
            dialog.musteriEkle(metin);
        }

        if (dialog.exec() == QDialog::Accepted) {
            KiralamaSozlesmesi ks;
            ks.sozlesme_id = sozlesmeIdSayaci++;
            ks.plaka = dialog.getPlaka().toStdString();
            ks.tc_no = dialog.getTcNo().toStdString();
            ks.baslangic_tarihi = dialog.getBaslangicTarihi().toStdString();
            ks.bitis_tarihi = dialog.getBitisTarihi().toStdString();
            ks.toplam_tutar = dialog.getToplamTutar();
            
            if (sozlesmeDeposu.ekle(ks.sozlesme_id, ks)) {
                auto opt = aracDeposu.bul(ks.plaka);
                if (opt.has_value()) {
                    Arac a = opt.value();
                    a.durum = AracDurum::Kirada;
                    aracDeposu.sil(ks.plaka);
                    aracDeposu.ekle(ks.plaka, a);
                    int aracSatir = aracModel->plakaIleSatirBul(ks.plaka);
                    if (aracSatir >= 0) aracModel->satirGuncelle(aracSatir, a);
                }

                kiralamaModel->satirEkle(ks);
                grafikGuncelle();
                statusBar()->showMessage("Kiralama sozlesmesi basariyla olusturuldu.", 3000);
            } else {
                statusBar()->showMessage("Hata: Sozlesme olusturulamadi!", 3000);
            }
        }
    });

    // ========== 4. GÖSTERGE PANELİ SEKMESİ ==========
    QWidget *dashboardSekmesi = new QWidget();
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardSekmesi);
    
    aracDurumSerisi = new QPieSeries();
    aracDurumGrafik = new QChart();
    aracDurumGrafik->addSeries(aracDurumSerisi);
    aracDurumGrafik->setTitle("Araç Durum Oranları");
    
    QChartView *chartView = new QChartView(aracDurumGrafik);
    chartView->setRenderHint(QPainter::Antialiasing);
    dashboardLayout->addWidget(chartView);

    // ========== 5. ARKA PLAN İŞLEMLERİ SEKMESİ (Faz 3) ==========
    QWidget *arkaplanSekmesi = new QWidget();
    arkaplanSekmesiOlustur(arkaplanSekmesi);

    // Sekmeleri ekle
    sekmePenceresi->addTab(araclarSekmesi, "Araclar");
    sekmePenceresi->addTab(musterilerSekmesi, "Musteriler");
    sekmePenceresi->addTab(kiralamaSekmesi, "Kiralama Sozlesmeleri");
    sekmePenceresi->addTab(dashboardSekmesi, "Gosterge Paneli");
    sekmePenceresi->addTab(arkaplanSekmesi, "Arka Plan Islemleri");

    setCentralWidget(sekmePenceresi);
}


// Alt kısımdaki durum çubuğunu oluşturan fonksiyon
// Araç istatistiklerini gösteren kalıcı bir etiket ekliyor
void MainWindow::durumCubuguOlustur() {
    durumEtiketi = new QLabel(" Durum: Musait (0) | Kirada (0) | Bakimda (0) ", this);
    statusBar()->addPermanentWidget(durumEtiketi);
    statusBar()->showMessage("Sistem basariyla yuklendi.", 3000);
}

// Tüm verileri kullanıcının seçtiği JSON dosyasına kaydeden fonksiyon
// Araçlar, müşteriler ve sözleşmeleri JSON formatında dışa aktarır
void MainWindow::verileriKaydetJSON() {
    QJsonObject kokObje;

    QJsonArray aracDizisi;
    for (const auto& [plaka, arac] : aracDeposu.tumunu_al()) {
        QJsonObject o;
        o["plaka"] = QString::fromStdString(arac.plaka);
        o["marka"] = QString::fromStdString(arac.marka);
        o["model"] = QString::fromStdString(arac.model);
        o["yil"] = arac.yil;
        o["yakit_tipi"] = QString::fromStdString(arac.yakit_tipi);
        o["gunluk_ucret"] = arac.gunluk_ucret;
        o["durum"] = static_cast<int>(arac.durum);
        aracDizisi.append(o);
    }
    kokObje["araclar"] = aracDizisi;

    QJsonArray musteriDizisi;
    for (const auto& [tc, musteri] : musteriDeposu.tumunu_al()) {
        QJsonObject o;
        o["tc_no"] = QString::fromStdString(musteri.tc_no);
        o["isim"] = QString::fromStdString(musteri.isim);
        o["soyisim"] = QString::fromStdString(musteri.soyisim);
        o["telefon"] = QString::fromStdString(musteri.telefon);
        o["ehliyet_no"] = QString::fromStdString(musteri.ehliyet_no);
        musteriDizisi.append(o);
    }
    kokObje["musteriler"] = musteriDizisi;

    QJsonArray sozlesmeDizisi;
    for (const auto& [id, ks] : sozlesmeDeposu.tumunu_al()) {
        QJsonObject o;
        o["sozlesme_id"] = ks.sozlesme_id;
        o["plaka"] = QString::fromStdString(ks.plaka);
        o["tc_no"] = QString::fromStdString(ks.tc_no);
        o["baslangic_tarihi"] = QString::fromStdString(ks.baslangic_tarihi);
        if (ks.bitis_tarihi.has_value()) {
            o["bitis_tarihi"] = QString::fromStdString(ks.bitis_tarihi.value());
        }
        o["toplam_tutar"] = ks.toplam_tutar;
        sozlesmeDizisi.append(o);
    }
    kokObje["sozlesmeler"] = sozlesmeDizisi;
    kokObje["sozlesmeIdSayaci"] = sozlesmeIdSayaci;

    QJsonDocument doc(kokObje);
    QString dosyaAdi = QFileDialog::getSaveFileName(this, "Verileri Kaydet", "", "JSON Dosyasi (*.json)");
    if (!dosyaAdi.isEmpty()) {
        QFile dosya(dosyaAdi);
        if (dosya.open(QIODevice::WriteOnly)) {
            dosya.write(doc.toJson());
            dosya.close();
            statusBar()->showMessage("Veriler basariyla disari aktarildi.", 3000);
        }
    }
}

// Kullanıcının seçtiği JSON dosyasından verileri okuyan fonksiyon
// Önce mevcut tüm depoları temizler, sonra JSON'dan okuduğu verileri yükler
// Son olarak tüm tabloları ve grafiği günceller
void MainWindow::verileriYukleJSON() {
    QString dosyaAdi = QFileDialog::getOpenFileName(this, "Verileri Yukle", "", "JSON Dosyasi (*.json)");
    if (dosyaAdi.isEmpty()) return;

    QFile dosya(dosyaAdi);
    if (!dosya.open(QIODevice::ReadOnly)) return;

    QByteArray data = dosya.readAll();
    dosya.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) return;

    QJsonObject kokObje = doc.object();

    aracDeposu.temizle();
    musteriDeposu.temizle();
    sozlesmeDeposu.temizle();

    QJsonArray aracDizisi = kokObje["araclar"].toArray();
    for (const QJsonValue& val : aracDizisi) {
        QJsonObject o = val.toObject();
        Arac a;
        a.plaka = o["plaka"].toString().toStdString();
        a.marka = o["marka"].toString().toStdString();
        a.model = o["model"].toString().toStdString();
        a.yil = o["yil"].toInt();
        a.yakit_tipi = o["yakit_tipi"].toString().toStdString();
        a.gunluk_ucret = o["gunluk_ucret"].toDouble();
        a.durum = static_cast<AracDurum>(o["durum"].toInt());
        aracDeposu.ekle(a.plaka, a);
    }

    QJsonArray musteriDizisi = kokObje["musteriler"].toArray();
    for (const QJsonValue& val : musteriDizisi) {
        QJsonObject o = val.toObject();
        Musteri m;
        m.tc_no = o["tc_no"].toString().toStdString();
        m.isim = o["isim"].toString().toStdString();
        m.soyisim = o["soyisim"].toString().toStdString();
        m.telefon = o["telefon"].toString().toStdString();
        m.ehliyet_no = o["ehliyet_no"].toString().toStdString();
        musteriDeposu.ekle(m.tc_no, m);
    }

    QJsonArray sozlesmeDizisi = kokObje["sozlesmeler"].toArray();
    for (const QJsonValue& val : sozlesmeDizisi) {
        QJsonObject o = val.toObject();
        KiralamaSozlesmesi ks;
        ks.sozlesme_id = o["sozlesme_id"].toInt();
        ks.plaka = o["plaka"].toString().toStdString();
        ks.tc_no = o["tc_no"].toString().toStdString();
        ks.baslangic_tarihi = o["baslangic_tarihi"].toString().toStdString();
        if (o.contains("bitis_tarihi")) {
            ks.bitis_tarihi = o["bitis_tarihi"].toString().toStdString();
        } else {
            ks.bitis_tarihi = std::nullopt;
        }
        ks.toplam_tutar = o["toplam_tutar"].toDouble();
        sozlesmeDeposu.ekle(ks.sozlesme_id, ks);
    }

    sozlesmeIdSayaci = kokObje["sozlesmeIdSayaci"].toInt(1);

    aracModel->verileriGuncelle();
    musteriModel->verileriGuncelle();
    kiralamaModel->verileriGuncelle();
    grafikGuncelle();

    statusBar()->showMessage("Veriler basariyla ice aktarildi.", 3000);
}

// Gösterge panelindeki pasta grafiğini güncelleyen fonksiyon
// Depodaki araçların durumlarını sayarak pasta dilimlerini oluşturuyor
// Ayrıca durum çubuğundaki istatistik metnini de güncelliyor
void MainWindow::grafikGuncelle() {
    if (!aracDurumSerisi) return;

    aracDurumSerisi->clear();
    
    int musait = 0, kirada = 0, bakimda = 0;
    for (const auto& [plaka, arac] : aracDeposu.tumunu_al()) {
        if (arac.durum == AracDurum::Musait) musait++;
        else if (arac.durum == AracDurum::Kirada) kirada++;
        else bakimda++;
    }

    if (musait == 0 && kirada == 0 && bakimda == 0) {
        aracDurumSerisi->append("Veri Yok", 1);
    } else {
        if (musait > 0) aracDurumSerisi->append("Musait", musait);
        if (kirada > 0) aracDurumSerisi->append("Kirada", kirada);
        if (bakimda > 0) aracDurumSerisi->append("Bakimda", bakimda);
    }

    QString durumText = QString(" Durum: Musait (%1) | Kirada (%2) | Bakimda (%3) ")
                        .arg(musait).arg(kirada).arg(bakimda);
    durumEtiketi->setText(durumText);
}



// =========================================================================
// Faz 3: Arka Plan İşlemleri - Sekme Oluşturma
// =========================================================================
// Arka plan işlemleri sekmesini oluşturan fonksiyon
// Fatura, rapor ve bakım butonları, ilerleme çubuğu ve sonuç alanı burada hazırlanıyor
void MainWindow::arkaplanSekmesiOlustur(QWidget *sekmesi) {
    QVBoxLayout *layout = new QVBoxLayout(sekmesi);

    // Başlık
    QLabel *baslik = new QLabel("Arka Plan Islemleri (Faz 3 - Cok Kanalli Programlama)");
    baslik->setStyleSheet("font-size: 14px; font-weight: bold; margin-bottom: 8px;");
    layout->addWidget(baslik);

    // İşlem butonları
    QHBoxLayout *butonLayout = new QHBoxLayout();
    QPushButton *faturaButonu = new QPushButton("Toplu Fatura Olustur");
    QPushButton *raporButonu = new QPushButton("Rapor Uret");
    QPushButton *bakimButonu = new QPushButton("Bakim Kontrolu");
    m_iptalButonu = new QPushButton("Iptal Et");
    m_iptalButonu->setEnabled(false);
    m_iptalButonu->setStyleSheet("background-color: #cc3333; color: white;");

    butonLayout->addWidget(faturaButonu);
    butonLayout->addWidget(raporButonu);
    butonLayout->addWidget(bakimButonu);
    butonLayout->addWidget(m_iptalButonu);
    butonLayout->addStretch();
    layout->addLayout(butonLayout);

    // İlerleme çubuğu
    m_ilerlemeCubugu = new QProgressBar();
    m_ilerlemeCubugu->setRange(0, 100);
    m_ilerlemeCubugu->setValue(0);
    m_ilerlemeCubugu->setTextVisible(true);
    m_ilerlemeCubugu->setFormat("%p% tamamlandi");
    layout->addWidget(m_ilerlemeCubugu);

    // Sonuç alanı
    m_sonucAlani = new QTextEdit();
    m_sonucAlani->setReadOnly(true);
    m_sonucAlani->setFont(QFont("Courier New", 10));
    m_sonucAlani->setPlaceholderText("Islem sonuclari burada gorunecek...");
    layout->addWidget(m_sonucAlani);

    // Sinyal/Slot bağlantıları
    connect(faturaButonu, &QPushButton::clicked, this, &MainWindow::faturaOlusturBaslat);
    connect(raporButonu, &QPushButton::clicked, this, &MainWindow::raporUretBaslat);
    connect(bakimButonu, &QPushButton::clicked, this, &MainWindow::bakimKontrolBaslat);
    connect(m_iptalButonu, &QPushButton::clicked, this, &MainWindow::islemIptalEt);
}

// =========================================================================
// Faz 3: Thread Başlatma Slotları
// =========================================================================

// Toplu fatura oluşturma işlemini arka planda başlatan fonksiyon
// Yeni bir thread ve işçi oluşturup sinyallerini bağlıyor
void MainWindow::faturaOlusturBaslat() {
    if (m_aktifThread && m_aktifThread->isRunning()) {
        QMessageBox::warning(this, "Uyari", "Zaten bir islem devam ediyor. Lutfen bekleyin veya iptal edin.");
        return;
    }

    m_sonucAlani->clear();
    m_ilerlemeCubugu->setValue(0);
    m_iptalButonu->setEnabled(true);

    // QThread ve Worker oluştur
    m_aktifThread = new QThread(this);
    m_faturaIsci = new FaturaOlusturucu(sozlesmeDeposu, aracDeposu, musteriDeposu, m_veriMutex);
    m_faturaIsci->moveToThread(m_aktifThread);

    // Sinyalleri bağla
    connect(m_aktifThread, &QThread::started, m_faturaIsci, &FaturaOlusturucu::calistir);
    connect(m_faturaIsci, &FaturaOlusturucu::ilerlemeGuncellendi, m_ilerlemeCubugu, &QProgressBar::setValue);
    connect(m_faturaIsci, &FaturaOlusturucu::tamamlandi, this, &MainWindow::islemTamamlandi);
    connect(m_faturaIsci, &FaturaOlusturucu::hataOlustu, this, &MainWindow::islemHatasi);
    connect(m_faturaIsci, &FaturaOlusturucu::tamamlandi, this, &MainWindow::threadTemizle);
    connect(m_faturaIsci, &FaturaOlusturucu::hataOlustu, this, &MainWindow::threadTemizle);

    statusBar()->showMessage("Toplu fatura olusturuluyor...", 0);
    m_aktifThread->start();
}

// Rapor üretme işlemini arka planda başlatan fonksiyon
void MainWindow::raporUretBaslat() {
    if (m_aktifThread && m_aktifThread->isRunning()) {
        QMessageBox::warning(this, "Uyari", "Zaten bir islem devam ediyor. Lutfen bekleyin veya iptal edin.");
        return;
    }

    m_sonucAlani->clear();
    m_ilerlemeCubugu->setValue(0);
    m_iptalButonu->setEnabled(true);

    m_aktifThread = new QThread(this);
    m_raporIsci = new RaporUretici(sozlesmeDeposu, aracDeposu, musteriDeposu, m_veriMutex);
    m_raporIsci->moveToThread(m_aktifThread);

    connect(m_aktifThread, &QThread::started, m_raporIsci, &RaporUretici::calistir);
    connect(m_raporIsci, &RaporUretici::ilerlemeGuncellendi, m_ilerlemeCubugu, &QProgressBar::setValue);
    connect(m_raporIsci, &RaporUretici::tamamlandi, this, &MainWindow::islemTamamlandi);
    connect(m_raporIsci, &RaporUretici::hataOlustu, this, &MainWindow::islemHatasi);
    connect(m_raporIsci, &RaporUretici::tamamlandi, this, &MainWindow::threadTemizle);
    connect(m_raporIsci, &RaporUretici::hataOlustu, this, &MainWindow::threadTemizle);

    statusBar()->showMessage("Kiralama raporu uretiliyor...", 0);
    m_aktifThread->start();
}

// Bakım kontrolü işlemini arka planda başlatan fonksiyon
void MainWindow::bakimKontrolBaslat() {
    if (m_aktifThread && m_aktifThread->isRunning()) {
        QMessageBox::warning(this, "Uyari", "Zaten bir islem devam ediyor. Lutfen bekleyin veya iptal edin.");
        return;
    }

    m_sonucAlani->clear();
    m_ilerlemeCubugu->setValue(0);
    m_iptalButonu->setEnabled(true);

    m_aktifThread = new QThread(this);
    m_bakimIsci = new BakimKontrolcu(aracDeposu, sozlesmeDeposu, m_veriMutex);
    m_bakimIsci->moveToThread(m_aktifThread);

    connect(m_aktifThread, &QThread::started, m_bakimIsci, &BakimKontrolcu::calistir);
    connect(m_bakimIsci, &BakimKontrolcu::ilerlemeGuncellendi, m_ilerlemeCubugu, &QProgressBar::setValue);
    connect(m_bakimIsci, &BakimKontrolcu::tamamlandi, this, &MainWindow::islemTamamlandi);
    connect(m_bakimIsci, &BakimKontrolcu::hataOlustu, this, &MainWindow::islemHatasi);
    connect(m_bakimIsci, &BakimKontrolcu::tamamlandi, this, &MainWindow::threadTemizle);
    connect(m_bakimIsci, &BakimKontrolcu::hataOlustu, this, &MainWindow::threadTemizle);

    statusBar()->showMessage("Arac bakim kontrolu yapiliyor...", 0);
    m_aktifThread->start();
}

// =========================================================================
// Faz 3: İptal, Tamamlanma ve Temizlik Slotları
// =========================================================================

// Çalışan arka plan işlemini iptal eden fonksiyon
// Hangi işçi aktifse onun iptal bayrağını aktif ediyor
void MainWindow::islemIptalEt() {
    if (m_faturaIsci) m_faturaIsci->iptalEt();
    if (m_raporIsci) m_raporIsci->iptalEt();
    if (m_bakimIsci) m_bakimIsci->iptalEt();
    statusBar()->showMessage("Iptal istegi gonderildi...", 3000);
}

// Arka plan işlemi başarıyla bitince çağrılır
// Sonuç metnini gösterir ve ilerleme çubuğunu %100'e çeker
void MainWindow::islemTamamlandi(const QString& sonuc) {
    m_sonucAlani->setText(sonuc);
    m_iptalButonu->setEnabled(false);
    m_ilerlemeCubugu->setValue(100);
    statusBar()->showMessage("Islem basariyla tamamlandi.", 3000);
}

// Arka plan işleminde hata olduğunda çağrılır
void MainWindow::islemHatasi(const QString& mesaj) {
    m_sonucAlani->setText("HATA: " + mesaj);
    m_iptalButonu->setEnabled(false);
    statusBar()->showMessage("Islem sirasinda hata olustu!", 3000);
}

// Thread ve işçi nesnelerini temizleyen fonksiyon
// İş bittikten sonra çağrılır, bellek sızıntısını önler
void MainWindow::threadTemizle() {
    if (m_aktifThread) {
        m_aktifThread->quit();
        m_aktifThread->wait();
        m_aktifThread->deleteLater();
        m_aktifThread = nullptr;
    }
    if (m_faturaIsci) { m_faturaIsci->deleteLater(); m_faturaIsci = nullptr; }
    if (m_raporIsci) { m_raporIsci->deleteLater(); m_raporIsci = nullptr; }
    if (m_bakimIsci) { m_bakimIsci->deleteLater(); m_bakimIsci = nullptr; }
}