#include "history.h"

void TrendBuffer::push(float value) {
    buf_[head_] = value;
    head_ = (head_ + 1) % kCapacity;
    if (count_ < kCapacity) count_++;
}

float TrendBuffer::at(int i) const {
    // O mais antigo esta em head_-count_ (mod kCapacity)
    const int start = (head_ - count_ + kCapacity) % kCapacity;
    return buf_[(start + i) % kCapacity];
}
