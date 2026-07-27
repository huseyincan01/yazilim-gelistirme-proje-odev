// Bu dosya müşteri verilerini tabloda göstermek için kullanılan model sınıfının tanımını içerir
// QAbstractTableModel'den türetilmiş, Depo'daki müşteri verilerini QTableView'a uygun formata çeviriyor

#pragma once
#include <QAbstractTableModel>
#include <vector>
#include "varliklar.h"
#include "depo.h"

// MusteriModel: Müşterileri tabloda göstermeye yarayan model sınıfı
// Depo'dan müşteri verilerini çekip tablonun anlayacağı formata çeviriyor
class MusteriModel : public QAbstractTableModel {
    Q_OBJECT

public:
    // Kurucu: müşteri deposunu referans olarak alır
    explicit MusteriModel(Depo<std::string, Musteri>& depo, QObject *parent = nullptr);

    // Tablodaki satır sayısını döndürür (kaç müşteri varsa o kadar satır)
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // Tablodaki sütun sayısını döndürür (5 sütun: TC, İsim, Soyisim, Telefon, Ehliyet)
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Hücredeki veriyi döndürür
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Sütun başlıklarını döndürür
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Depodan tüm müşterileri çekip tabloyu baştan oluşturur
    void verileriGuncelle();

    // Tabloya yeni bir müşteri satırı ekler
    void satirEkle(const Musteri& musteri);

    // Belirli bir satırı tablodan siler
    void satirSil(int satirIndeksi);

    // TC numarasına göre satır indeksini bulur, bulamazsa -1 döner
    int tcIleSatirBul(const std::string& tc_no) const;

private:
    Depo<std::string, Musteri>& m_depo; // Müşterilerin asıl tutulduğu depo referansı
    std::vector<Musteri> m_veriler;     // Tabloda göstermek için önbelleğe alınmış liste
};
