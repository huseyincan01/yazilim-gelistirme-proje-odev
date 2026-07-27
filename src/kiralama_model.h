// Bu dosya kiralama sözleşmelerini tabloda göstermek için kullanılan model sınıfının tanımını içerir
// QAbstractTableModel'den türetilmiş, Depo'daki sözleşme verilerini QTableView'a uygun formata çeviriyor
// Aktif ve biten sözleşmeleri farklı renklerle gösteriyor

#pragma once
#include <QAbstractTableModel>
#include <vector>
#include "varliklar.h"
#include "depo.h"

// KiralamaModel: Sözleşmeleri tabloda göstermeye yarayan model sınıfı
// Süresi dolmuş sözleşmeleri pembe, devam edenleri yeşil arka planla gösteriyor
class KiralamaModel : public QAbstractTableModel {
    Q_OBJECT

public:
    // Kurucu: sözleşme deposunu referans olarak alır
    explicit KiralamaModel(Depo<int, KiralamaSozlesmesi>& depo, QObject *parent = nullptr);

    // Tablodaki satır sayısını döndürür
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // Tablodaki sütun sayısını döndürür (6 sütun: ID, Plaka, TC, Başlangıç, Bitiş, Tutar)
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Hücredeki veriyi döndürür, arka plan rengini de buradan ayarlıyoruz
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Sütun başlıklarını döndürür
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Depodan tüm sözleşmeleri çekip tabloyu baştan oluşturur
    void verileriGuncelle();

    // Tabloya yeni bir sözleşme satırı ekler
    void satirEkle(const KiralamaSozlesmesi& sozlesme);

    // Belirli bir satırı tablodan siler
    void satirSil(int satirIndeksi);

    // Sözleşme ID'sine göre satır indeksini bulur, bulamazsa -1 döner
    int idIleSatirBul(int sozlesmeId) const;

private:
    Depo<int, KiralamaSozlesmesi>& m_depo; // Sözleşmelerin asıl tutulduğu depo referansı
    std::vector<KiralamaSozlesmesi> m_veriler; // Tabloda göstermek için önbelleğe alınmış liste
};
