// Bu dosya genel amaçlı bir şablon (template) depo sınıfı içerir
// Anahtar-değer çifti mantığıyla çalışır, yani her verinin bir kimliği (anahtarı) var
// Araçlar plakaya göre, müşteriler TC'ye göre, sözleşmeler ID'ye göre saklanır
// İçeride std::map kullanılıyor, bu sayede veriler her zaman sıralı tutuluyor

#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <vector>
#include <bits/ranges_algo.h>
#include <bits/ranges_algobase.h>


// Depo sınıfı: Anahtar ve Deger tiplerini template parametre olarak alır
// Mesela Depo<string, Arac> dersen plaka ile araç eşleştirilir
// Depo<int, KiralamaSozlesmesi> dersen ID ile sözleşme eşleştirilir
template <typename Anahtar, typename Deger>
class Depo {
public:
    // Depoya yeni bir kayıt ekler
    // Eğer aynı anahtarla zaten bir kayıt varsa ekleme yapmaz ve false döner
    // Başarılı olursa true döner
    bool ekle(const Anahtar& anahtar, const Deger& deger) {
        auto [it, eklendi] = m_veriler.insert({anahtar, deger});
        return eklendi;
    }

    // Verilen anahtara sahip kaydı depoda arar
    // Bulursa o kaydı optional içinde döndürür, bulamazsa nullopt döner
    // const olduğu için depoyu değiştirmez, sadece okur
    std::optional<Deger> bul(const Anahtar& anahtar) const {
        auto x = m_veriler.find(anahtar);
        if (x != m_veriler.end()) {
            return x->second; // it->first anahtardır, it->second ise değerdir
        }
        return std::nullopt;
    }

    // Verilen anahtara sahip kaydı depodan siler
    // Silme başarılıysa true, kayıt bulunamadıysa false döner
    bool sil(const Anahtar& anahtar) {
        return m_veriler.erase(anahtar);
        // erase silinen öğre sayısını döndürür, silerse (multi olmayan map için) 1 silmezse 0 döner
    }

    // Depodaki tüm kayıtları map referansı olarak döndürür
    // const referans olduğu için dışarıdan değiştirilemez, sadece okunabilir
    const std::map<Anahtar, Deger>& tumunu_al() const {
        return m_veriler;
    }

    // Depoda kaç tane kayıt olduğunu döndürür
    std::size_t boyut() const {
        return m_veriler.size();
    }

    // Depodaki tüm kayıtları siler, depoyu sıfırlar
    void temizle() {
        m_veriler.clear();
    }

    // Belirli bir koşula uyan kayıtları filtreleyip vektör olarak döndürür
    // Parametre olarak bir lambda ya da fonksiyon alır
    // Mesela durumu "Musait" olan araçları bulmak için kullanılır
    // İçeride std::ranges kullanılıyor, modern C++20 özelliği
    std::vector<Deger> filtrele(std::function<bool(const Deger&)> kosul) const
    {
        std::vector<Deger> sonuc;
        for (const auto& deger : std::views::filter(std::ranges::views::values(m_veriler), kosul)){
            sonuc.push_back(deger);
        }
        return sonuc;
    }

private:
    // Verilerin gerçekten tutulduğu yer
    // map sayesinde anahtarlar otomatik olarak sıralı tutuluyor
    std::map<Anahtar, Deger> m_veriler;
};
