#pragma once
// Buffer circular da janela de tendencia (FR-UI-003 | decisao D-003: 120 pontos).
#include <stdint.h>

class TrendBuffer {
public:
    static constexpr int kCapacity = 120;

    void push(float value);
    int size() const { return count_; }
    // Indice 0 = amostra mais antiga; indice size()-1 = mais recente.
    float at(int i) const;

private:
    float buf_[kCapacity];
    int head_ = 0;   // proxima posicao de escrita
    int count_ = 0;
};
