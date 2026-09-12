// trend_buffer.cpp — implementação do histórico circular.
#include "trend_buffer.h"

namespace thermal {

TrendBuffer::TrendBuffer(int16_t* storage, size_t capacity)
    : buf_(storage), cap_(capacity) {}

void TrendBuffer::clear() {
  head_ = 0;
  count_ = 0;
}

void TrendBuffer::push(int16_t value) {
  if (cap_ == 0) {
    return;
  }
  buf_[head_] = value;
  head_ = (head_ + 1) % cap_;
  if (count_ < cap_) {
    ++count_;
  }
}

int16_t TrendBuffer::at(size_t index) const {
  if (index >= count_ || cap_ == 0) {
    return 0;
  }
  const size_t start = (count_ == cap_) ? head_ : 0;  // cheio: mais antigo = head_
  return buf_[(start + index) % cap_];
}

int16_t TrendBuffer::newest() const {
  return (count_ > 0) ? at(count_ - 1) : 0;
}

}  // namespace thermal
