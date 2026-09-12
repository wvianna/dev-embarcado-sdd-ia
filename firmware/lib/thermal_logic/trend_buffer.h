// trend_buffer.h — histórico circular de temperatura para o gráfico (lógica pura).
//
// Owner da memória é o chamador (buffers estáticos; sem alocação dinâmica no alvo).
// `at(0)` é a amostra mais antiga e `at(size()-1)` a mais recente (ordem do gráfico).
// Capacidade definida em config.h (kTrendCapacity = 120 => 2 min a 1 s — design §3.4).
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace thermal {

class TrendBuffer {
 public:
  TrendBuffer(int16_t* storage, size_t capacity);

  void clear();
  void push(int16_t value);  // ao encher, sobrescreve a amostra mais antiga
  size_t size() const { return count_; }
  size_t capacity() const { return cap_; }
  int16_t at(size_t index) const;  // 0 = mais antiga; fora de faixa => 0
  int16_t newest() const;

 private:
  int16_t* buf_;
  size_t cap_;
  size_t head_ = 0;   // próximo ponto de escrita
  size_t count_ = 0;  // amostras válidas armazenadas (<= cap_)
};

}  // namespace thermal
