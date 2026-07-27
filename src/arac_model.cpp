// Bu dosya AracModel sınıfının implementasyonunu içerir
// Araç tablosunun verileri nasıl gösterileceği, satır ekleme/silme/güncelleme
// işlemlerinin nasıl yapılacağı burada kodlanmış

#include "arac_model.h"
#include <QColor>

// Kurucu: depo referansını alıp ilk verileri yüklüyor
AracModel::AracModel(Depo<std::string, Arac>& depo, QObject *parent)
    : QAbstractTableModel(parent), m_depo(depo) {
    verileriGuncelle();
}

// Tabloda kaç satır olduğunu söyler, yani kaç araç gösteriyoruz
int AracModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_veriler.size();
}

// Tabloda kaç sütun olduğunu söyler
// 7 tane sütun var: Plaka, Marka, Model, Yil, Yakit, Ucret, Durum
int AracModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return 7;
}

// Tablonun her bir hücresine ne yazılacağını belirleyen fonksiyon
// DisplayRole: hücredeki metin, BackgroundRole: arka plan rengi, ForegroundRole: yazı rengi
// Duruma göre satırlar renklendirilir: yeşil musait, kırmızı kirada, sarı bakımda
QVariant AracModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_veriler.size()))
        return QVariant();

    if (role == Qt::DisplayRole) {
        const Arac& a = m_veriler[index.row()];
        switch (index.column()) {
            case 0: return QString::fromStdString(a.plaka);
            case 1: return QString::fromStdString(a.marka);
            case 2: return QString::fromStdString(a.model);
            case 3: return a.yil;
            case 4: return QString::fromStdString(a.yakit_tipi);
            case 5: return QString("%1 TL").arg(a.gunluk_ucret);
            case 6: return QString::fromStdString(durumYazisi(a.durum));
            default: return QVariant();
        }
    }
    
    // Yazı rengini koyu tutuyoruz ki arka plan renkleriyle kontrast oluşsun
    if (role == Qt::ForegroundRole) {
        return QColor(30, 30, 30);
    }

    // Araç durumuna göre satırın arka plan rengini belirliyoruz
    if (role == Qt::BackgroundRole) {
        const Arac& a = m_veriler[index.row()];
        if (a.durum == AracDurum::Musait) {
            return QColor(170, 255, 170); // Açık yeşil: araç müsait
        } else if (a.durum == AracDurum::Kirada) {
            return QColor(255, 170, 170); // Açık kırmızı: araç kirada
        } else {
            return QColor(255, 230, 140); // Açık sarı: araç bakımda
        }
    }

    return QVariant();
}

// Tablonun üstündeki sütun başlıklarını döndürür
QVariant AracModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "Plaka";
            case 1: return "Marka";
            case 2: return "Model";
            case 3: return "Yil";
            case 4: return "Yakit Tipi";
            case 5: return "Gunluk Ucret";
            case 6: return "Durum";
            default: return QVariant();
        }
    }
    return QVariant();
}

// Depodan tüm araçları çekip tabloyu baştan oluşturuyor
// beginResetModel ve endResetModel arasında yapılıyor ki tablo düzgün güncellensin
void AracModel::verileriGuncelle() {
    beginResetModel();
    m_veriler.clear();
    for (const auto& [plaka, arac] : m_depo.tumunu_al()) {
        m_veriler.push_back(arac);
    }
    endResetModel();
}

// Tabloya yeni bir araç satırı ekler
// beginInsertRows/endInsertRows çağrılarak tabloya "yeni satır geliyor" haberi verilir
void AracModel::satirEkle(const Arac& arac) {
    int yeniSatir = m_veriler.size();
    beginInsertRows(QModelIndex(), yeniSatir, yeniSatir);
    m_veriler.push_back(arac);
    endInsertRows();
}

// Belirtilen indeksteki satırı tablodan kaldırır
// Geçersiz indeks gelirse sessizce hiçbir şey yapmaz
void AracModel::satirSil(int satirIndeksi) {
    if (satirIndeksi < 0 || satirIndeksi >= static_cast<int>(m_veriler.size())) return;
    beginRemoveRows(QModelIndex(), satirIndeksi, satirIndeksi);
    m_veriler.erase(m_veriler.begin() + satirIndeksi);
    endRemoveRows();
}

// Belirtilen satırdaki aracın bilgilerini günceller
// dataChanged sinyali yayarak tablonun o satırı yeniden çizmesini sağlar
void AracModel::satirGuncelle(int satirIndeksi, const Arac& arac) {
    if (satirIndeksi < 0 || satirIndeksi >= static_cast<int>(m_veriler.size())) return;
    m_veriler[satirIndeksi] = arac;
    emit dataChanged(index(satirIndeksi, 0), index(satirIndeksi, columnCount() - 1));
}

// Verilen plakaya sahip aracın tabloda kaçıncı satırda olduğunu arar
// Baştan sona tarar, bulursa indeksi döner, bulamazsa -1 döner
int AracModel::plakaIleSatirBul(const std::string& plaka) const {
    for (int i = 0; i < static_cast<int>(m_veriler.size()); ++i) {
        if (m_veriler[i].plaka == plaka) return i;
    }
    return -1;
}
