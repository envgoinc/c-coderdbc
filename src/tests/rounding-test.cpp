#include "rounding.hpp"
#include "testapi.h"

TEST(SignalRounding, ConversionMacros)
{
  // Calls must work even without USE_SIGFLOAT, and evaluate x only once.
  sigfloat_t voltage = 48.1;
  expect_eq(ROUNDING_Voltage_ro_toS(voltage), 481);
  expect_eq(ROUNDING_Voltage_ro_toS((sigfloat_t)48.16), 482);
  expect_eq(ROUNDING_Current_ro_toS((sigfloat_t)-12.36), -124);
  expect_eq(ROUNDING_Current_ro_toS((sigfloat_t)-12.34), -123);
  expect_eq(ROUNDING_Current_ro_toS((sigfloat_t)0), 0);
  expect_eq(ROUNDING_Voltage_ro_toS((sigfloat_t)409.5), 4095);
  expect_eq(ROUNDING_Current_ro_toS((sigfloat_t)204.7), 2047);
  expect_eq(ROUNDING_Current_ro_toS((sigfloat_t)-204.8), -2048);
  // Exactly representable halfway cases, including negative factor and offset.
  expect_eq(ROUNDING_Offset_ro_toS((sigfloat_t)-40), 1);
  expect_eq(ROUNDING_Offset_ro_toS((sigfloat_t)-40.5), -1);
  expect_eq(ROUNDING_NegativeFactor_ro_toS((sigfloat_t)2), 1);
  expect_eq(ROUNDING_NegativeFactor_ro_toS((sigfloat_t)2.5), -1);
  expect_eq(ROUNDING_Voltage_ro_toS(voltage++), 481);
  EXPECT_NEAR(voltage, 49.1, 0.00001);
  // Preserve exact integer-offset conversion and integer-division behavior.
  expect_eq(ROUNDING_Temperature_ro_toS(-40), 0);
  expect_eq(ROUNDING_Temperature_ro_toS(215), 255);
  expect_eq(ROUNDING_Scaled_ro_toS(3), 1);
}

TEST(SignalRounding, GeneratedPackAndUnpack)
{
  Rounding_t message{};
#ifdef ROUNDING_USE_SIGFLOAT
  message.Voltage_phys = 48.1;
  message.Current_phys = -12.36;
  message.Offset_phys = -40.5;
  message.NegativeFactor_phys = 2;
#else
  message.Voltage_ro = 481;
  message.Current_ro = -124;
  message.Offset_ro = -1;
  message.NegativeFactor_ro = 1;
#endif
  uint8_t data[8]{}, length{}, extended{};
  Pack_Rounding_rounding(&message, data, &length, &extended);
  const uint8_t expected[] = {0xe1, 0x01, 0x84, 0xff, 0xff, 0xff, 0x01, 0x00};
  expect_eq(length, 8);
  expect_eq(extended, 0);
  for (unsigned i = 0; i < 8; ++i) { expect_eq(data[i], expected[i]); }
  Rounding_t decoded{};
  Unpack_Rounding_rounding(&decoded, data, length);
  expect_eq(decoded.Voltage_ro, 481);
  expect_eq(decoded.Current_ro, -124);
  expect_eq(decoded.Offset_ro, -1);
  expect_eq(decoded.NegativeFactor_ro, 1);
#ifdef ROUNDING_USE_SIGFLOAT
  EXPECT_NEAR(decoded.Voltage_phys, 48.1, 0.00001);
  EXPECT_NEAR(decoded.Current_phys, -12.4, 0.00001);
  expect_eq(decoded.Offset_phys, -40.75);
  expect_eq(decoded.NegativeFactor_phys, 1.75);
#endif
}

TEST(SignalRounding, TenthsRoundTrip)
{
  // Sweep the STM controller's 12-bit electrical ranges, including sentinels.
  for (unsigned raw = 0; raw <= 4095; ++raw)
  {
    const sigfloat_t physical = ROUNDING_Voltage_ro_fromS(raw);
    expect_eq(ROUNDING_Voltage_ro_toS(physical), raw);
  }
  for (int raw = -2048; raw <= 2047; ++raw)
  {
    const sigfloat_t physical = ROUNDING_Current_ro_fromS(raw);
    expect_eq(ROUNDING_Current_ro_toS(physical), raw);
  }
}
