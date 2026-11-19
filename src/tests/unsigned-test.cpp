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

TEST(UnsignedCodec, SignalRangeFromDbc)
{
  // Physical [min|max] from the DBC, named after the DBC signal.
  expect_eq(UnsignedStatus_Speed_MIN, 0);
  expect_eq(UnsignedStatus_Speed_MAX, 255);
  expect_eq(UnsignedStatus_Temperature_MIN, -40);
  expect_eq(UnsignedStatus_Temperature_MAX, 215);
#if defined(Unranged_Raw_MIN) || defined(Unranged_Raw_MAX)
  FAIL() << "[0|0] means no range and must not emit MIN/MAX";
#endif
  // Codex review on c-coderdbc #1: Clash.Sig_X and Clash_Sig.X both flatten to Clash_Sig_X; neither
  // may be emitted, or one would silently take the other's limit.
#if defined(Clash_Sig_X_MIN) || defined(Clash_Sig_X_MAX)
  FAIL() << "an ambiguous range name must not be emitted";
#endif
  // Codex review on c-coderdbc #1: signal Vt in message Vt has values "MIN" and "MAX"; the
  // value-table defines keep their values and the range [1|2] is not emitted over them.
  expect_eq(Vt_Vt_MIN, 0);
  expect_eq(Vt_Vt_MAX, 3);
  // Codex review on c-coderdbc #1: message VER, signal UNSIGNED would make VER_UNSIGNED_MIN, the
  // driver's minor-version define; the version keeps its value and the range [3|9] is not emitted.
  expect_eq(VER_UNSIGNED_MIN, 0);
#if defined(VER_UNSIGNED_MAX)
  FAIL() << "a range clashing with the version define must not be emitted";
#endif
}
