// Bu dosya MusteriModel sınıfının implementasyonunu içerir
// Müşteri tablosunun verileri nasıl gösterileceği, satır ekleme/silme
// işlemlerinin nasıl yapılacağı burada kodlanmış

#include "musteri_model.h"

// Kurucu: müşteri deposunu alıp ilk verileri yüklüyor
MusteriModel::MusteriModel(Depo<std::string, Musteri>& depo, QObject *parent)
    : QAbstractTableModel(parent), m_depo(depo) {
    verileriGuncelle();
}

// Tablodaki satır sayısını döndürür
int MusteriModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_veriler.size();
}

// Tablodaki sütun sayısını döndürür
// 5 sütun var: TC, İsim, Soyisim, Telefon, Ehliyet
int MusteriModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return 5;
}

// Her hücreye ne yazılacağını belirleyen fonksiyon
// Sütun numarasına göre müşterinin ilgili bilgisini döndürür
QVariant MusteriModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_veriler.size()))
        return QVariant();

    if (role == Qt::DisplayRole) {
        const Musteri& m = m_veriler[index.row()];
        switch (index.column()) {
            case 0: return QString::fromStdString(m.tc_no);
            case 1: return QString::fromStdString(m.isim);
            case 2: return QString::fromStdString(m.soyisim);
            case 3: return QString::fromStdString(m.telefon);
            case 4: return QString::fromStdString(m.ehliyet_no);
            default: return QVariant();
        }
    }
    return QVariant();
}

// Sütun başlıklarını döndürür
QVariant MusteriModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "TC Kimlik No";
            case 1: return "Isim";
            case 2: return "Soyisim";
            case 3: return "Telefon";
            case 4: return "Ehliyet No";
            default: return QVariant();
        }
    }
    return QVariant();
}

// Depodan tüm müşterileri çekip tabloyu sıfırdan oluşturur
void MusteriModel::verileriGuncelle() {
    beginResetModel();
    m_veriler.clear();
    for (const auto& [tc, musteri] : m_depo.tumunu_al()) {
        m_veriler.push_back(musteri);
    }
    endResetModel();
}

// Tabloya yeni bir müşteri satırı ekler
void MusteriModel::satirEkle(const Musteri& musteri) {
    int yeniSatir = m_veriler.size();
    beginInsertRows(QModelIndex(), yeniSatir, yeniSatir);
    m_veriler.push_back(musteri);
    endInsertRows();
}

// Belirtilen satırı tablodan kaldırır
void MusteriModel::satirSil(int satirIndeksi) {
    if (satirIndeksi < 0 || satirIndeksi >= static_cast<int>(m_veriler.size())) return;
    beginRemoveRows(QModelIndex(), satirIndeksi, satirIndeksi);
    m_veriler.erase(m_veriler.begin() + satirIndeksi);
    endRemoveRows();
}

// Verilen TC numarasına sahip müşterinin tabloda kaçıncı satırda olduğunu arar
int MusteriModel::tcIleSatirBul(const std::string& tc_no) const {
    for (int i = 0; i < static_cast<int>(m_veriler.size()); ++i) {
        if (m_veriler[i].tc_no == tc_no) return i;
    }
    return -1;
}
