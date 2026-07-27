// Bu dosya KiralamaModel sınıfının implementasyonunu içerir
// Kiralama sözleşmelerinin tabloda nasıl gösterileceği burada kodlanmış
// Bitmiş sözleşmeler pembe, devam eden sözleşmeler yeşil arka planla boyanıyor

#include "kiralama_model.h"
#include <QColor>
#include <QDate>

// Kurucu: sözleşme deposunu alıp ilk verileri yüklüyor
KiralamaModel::KiralamaModel(Depo<int, KiralamaSozlesmesi>& depo, QObject *parent)
    : QAbstractTableModel(parent), m_depo(depo) {
    verileriGuncelle();
}

// Tablodaki satır sayısını döndürür
int KiralamaModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_veriler.size();
}

// Tablodaki sütun sayısını döndürür: ID, Plaka, TC, Başlangıç, Bitiş, Tutar
int KiralamaModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return 6;
}

// Her hücredeki veriyi döndüren fonksiyon
// Bitiş tarihi yoksa "Devam Ediyor" yazıyor
// Arka plan rengi ile bitmiş ve devam eden sözleşmeleri görsel olarak ayırıyor
QVariant KiralamaModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_veriler.size()))
        return QVariant();

    const KiralamaSozlesmesi& ks = m_veriler[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return ks.sozlesme_id;
        case 1: return QString::fromStdString(ks.plaka);
        case 2: return QString::fromStdString(ks.tc_no);
        case 3: return QString::fromStdString(ks.baslangic_tarihi);
        case 4: return ks.bitis_tarihi.has_value()
                       ? QString::fromStdString(ks.bitis_tarihi.value())
                       : QString("Devam Ediyor");
        case 5: return QString("%1 TL").arg(ks.toplam_tutar);
        default: return QVariant();
        }
    }
    // Süresi geçmiş sözleşmeleri pembe, devam edenleri yeşil boyuyoruz
    else if (role == Qt::BackgroundRole) {
        if (ks.bitis_tarihi.has_value()) {
            QDate bitis = QDate::fromString(
                QString::fromStdString(ks.bitis_tarihi.value()),
                "yyyy-MM-dd"
                );
            if (bitis.isValid() && bitis < QDate::currentDate()) {
                return QColor(255, 200, 200); // Pembe: süresi dolmuş
            }
        }
        return QColor(0, 170, 0); // Yeşil: aktif sözleşme
    }
    else if (role == Qt::ForegroundRole) {
        return QColor(Qt::black);
    }

    return QVariant();
}

// Sütun başlıklarını döndürür
QVariant KiralamaModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "Sozlesme ID";
            case 1: return "Plaka";
            case 2: return "TC Kimlik No";
            case 3: return "Alis Tarihi";
            case 4: return "Teslim Tarihi";
            case 5: return "Toplam Tutar";
            default: return QVariant();
        }
    }
    return QVariant();
}

// Depodan tüm sözleşmeleri çekip tabloyu sıfırdan yeniden oluşturur
void KiralamaModel::verileriGuncelle() {
    beginResetModel();
    m_veriler.clear();
    for (const auto& [id, sozlesme] : m_depo.tumunu_al()) {
        m_veriler.push_back(sozlesme);
    }
    endResetModel();
}

// Tabloya yeni bir sözleşme satırı ekler
void KiralamaModel::satirEkle(const KiralamaSozlesmesi& sozlesme) {
    int yeniSatir = m_veriler.size();
    beginInsertRows(QModelIndex(), yeniSatir, yeniSatir);
    m_veriler.push_back(sozlesme);
    endInsertRows();
}

// Belirtilen indeksteki satırı tablodan kaldırır
void KiralamaModel::satirSil(int satirIndeksi) {
    if (satirIndeksi < 0 || satirIndeksi >= static_cast<int>(m_veriler.size())) return;
    beginRemoveRows(QModelIndex(), satirIndeksi, satirIndeksi);
    m_veriler.erase(m_veriler.begin() + satirIndeksi);
    endRemoveRows();
}

// Verilen sözleşme ID'sine sahip kaydın tabloda kaçıncı satırda olduğunu arar
int KiralamaModel::idIleSatirBul(int sozlesmeId) const {
    for (int i = 0; i < static_cast<int>(m_veriler.size()); ++i) {
        if (m_veriler[i].sozlesme_id == sozlesmeId) return i;
    }
    return -1;
}
