// Bu dosya araç verilerini tabloda göstermek için kullanılan model sınıfının tanımını içerir
// Qt'nin Model/View mimarisine uygun şekilde QAbstractTableModel'den türetilmiş
// Depo'daki araç verilerini alıp QTableView'a uygun formata çeviriyor

#pragma once
#include <QAbstractTableModel>
#include <vector>
#include "varliklar.h"
#include "depo.h"

// AracModel: Araçları tabloda göstermeye yarayan model sınıfı
// Depo'dan verileri çekip tablonun anlayacağı dile çeviriyor
// Satır ekleme, silme ve güncelleme işlemlerini de buradan yönetiyoruz
class AracModel : public QAbstractTableModel {
    Q_OBJECT

public:
    // Kurucu: araç deposunu referans olarak alır
    explicit AracModel(Depo<std::string, Arac>& depo, QObject *parent = nullptr);

    // Tablodaki satır sayısını döndürür (kaç araç varsa o kadar satır)
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // Tablodaki sütun sayısını döndürür (7 sütun: Plaka, Marka, Model, Yıl, Yakıt, Ücret, Durum)
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Tablonun belirli bir hücresindeki veriyi döndürür
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Tablonun sütun başlıklarını döndürür
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Depodan tüm verileri tekrar çekip tabloyu sıfırdan yeniden çizer
    void verileriGuncelle();

    // Tabloya tek bir araç satırı ekler
    void satirEkle(const Arac& arac);

    // Belirli bir satırı tablodan siler
    void satirSil(int satirIndeksi);

    // Belirli bir satırın verisini günceller
    void satirGuncelle(int satirIndeksi, const Arac& arac);

    // Plakaya göre satır indeksini bulur, bulamazsa -1 döner
    int plakaIleSatirBul(const std::string& plaka) const;

private:
    Depo<std::string, Arac>& m_depo; // Araçların asıl tutulduğu depo referansı
    std::vector<Arac> m_veriler;     // Tabloda göstermek için önbelleğe alınmış liste
};
