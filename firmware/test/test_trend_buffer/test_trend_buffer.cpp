// Testes HOST do histórico circular do gráfico (env native).
#include <unity.h>

#include "trend_buffer.h"

using thermal::TrendBuffer;

namespace {
int16_t storage[4] = {0, 0, 0, 0};
TrendBuffer buffer(storage, 4);
}

void setUp() {
  buffer.clear();
  for (int i = 0; i < 4; ++i) {
    storage[i] = 0;
  }
}
void tearDown() {}

void test_empty_buffer() {
  TEST_ASSERT_EQUAL_UINT(0, static_cast<unsigned>(buffer.size()));
  TEST_ASSERT_EQUAL_INT16(0, buffer.newest());
  TEST_ASSERT_EQUAL_INT16(0, buffer.at(0));
}

void test_push_order_until_full() {
  buffer.push(100);
  buffer.push(200);
  buffer.push(300);
  TEST_ASSERT_EQUAL_UINT(3, static_cast<unsigned>(buffer.size()));
  TEST_ASSERT_EQUAL_INT16(100, buffer.at(0));  // mais antiga
  TEST_ASSERT_EQUAL_INT16(200, buffer.at(1));
  TEST_ASSERT_EQUAL_INT16(300, buffer.at(2));  // mais recente
  TEST_ASSERT_EQUAL_INT16(300, buffer.newest());
}

void test_overwrites_oldest_when_full() {
  buffer.push(1);
  buffer.push(2);
  buffer.push(3);
  buffer.push(4);
  buffer.push(5);  // descarta a amostra 1
  TEST_ASSERT_EQUAL_UINT(4, static_cast<unsigned>(buffer.size()));
  TEST_ASSERT_EQUAL_INT16(2, buffer.at(0));
  TEST_ASSERT_EQUAL_INT16(3, buffer.at(1));
  TEST_ASSERT_EQUAL_INT16(4, buffer.at(2));
  TEST_ASSERT_EQUAL_INT16(5, buffer.at(3));
  TEST_ASSERT_EQUAL_INT16(5, buffer.newest());
}

void test_out_of_range_returns_zero() {
  TEST_ASSERT_EQUAL_INT16(0, buffer.at(0));
  buffer.push(7);
  TEST_ASSERT_EQUAL_INT16(0, buffer.at(1));
  TEST_ASSERT_EQUAL_INT16(0, buffer.at(99));
}

void test_zero_capacity_is_safe() {
  TrendBuffer empty(nullptr, 0);
  empty.push(1);
  empty.clear();
  TEST_ASSERT_EQUAL_UINT(0, static_cast<unsigned>(empty.size()));
  TEST_ASSERT_EQUAL_INT16(0, empty.at(0));
  TEST_ASSERT_EQUAL_INT16(0, empty.newest());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_empty_buffer);
  RUN_TEST(test_push_order_until_full);
  RUN_TEST(test_overwrites_oldest_when_full);
  RUN_TEST(test_out_of_range_returns_zero);
  RUN_TEST(test_zero_capacity_is_safe);
  return UNITY_END();
}
