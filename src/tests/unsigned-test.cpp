#include "unsigned.hpp"
#include "testapi.h"

TEST(UnsignedCodec, PackAndUnpack)
{
  UnsignedStatus_t message{};
  message.Speed = 200;
  message.Temperature_ro = 0; // Unsigned wire signal, physical value -40.
  uint8_t data[8]{}, length{}, extended{};
  Pack_UnsignedStatus_unsigned(&message, data, &length, &extended);
  expect_eq(length, 2);
  expect_eq(extended, 0);
  expect_eq(data[0], 200);
  expect_eq(data[1], 0);
  UnsignedStatus_t decoded{};
  Unpack_UnsignedStatus_unsigned(&decoded, data, length);
  expect_eq(decoded.Speed, 200);
  expect_eq(decoded.Temperature_ro, 0);
  expect_eq(UNSIGNED_Temperature_ro_fromS(decoded.Temperature_ro), -40);
}
