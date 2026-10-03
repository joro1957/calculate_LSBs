#pragma once
#include <Arduino.h>

/// @brief Calculates the multiplier for a given resistor voltage divider.
/// @param R_high Value in ohms of high-side resistor(s).
/// @param R_low Value in ohms of low-side resistor(s).
/// @return Multiplier to calculate the original input voltage from the divided voltage. Will return 1.0 if either input is NaN or <=0.
inline float calcMultiplier_vDivider(float R_high, float R_low) {
  if (R_high <= 0.0f || R_low <= 0.0f || isnan(R_high) || isnan(R_low)) {
    return 1.0f;
  }
  const float sum = R_high + R_low;
  return sum / R_low;
}

/// @brief Calculates the multiplier from an incorrect ADC voltage read & an external measurement/multimeter.
/// @param adc_voltage  Incorrect ADC read voltage.
/// @param ext_voltage Known accurate externally measured voltage, such as a calibrated multimeter.
/// @return Multiplier to calculate the original input voltage from the divided voltage. Will return 1.0 if either input is NaN or <=0.
inline float calcMultiplier_externalVoltage(float adc_voltage, float ext_voltage) {
  if (adc_voltage <= 0.0f || ext_voltage <= 0.0f || isnan(adc_voltage) || isnan(ext_voltage)) {
    return 1.0f;
  }
  return ext_voltage / adc_voltage;
}

/// @brief Calculates the multiplier from an incorrect ADC voltage read that was already multiplied & an external measurement/multimeter.
/// @param existing_multiplier Multiplier that was used to get incorrect adc_voltage.
/// @param adc_voltage  Incorrect ADC read voltage, already multiplied.
/// @param ext_voltage Known accurate externally measured voltage, such as a calibrated multimeter.
/// @return Multiplier to calculate the original input voltage from the adc_voltage. Will return 1.0 if either input is NaN or <=0.
inline float calcMultiplier_multiplied_externalVoltage(float existing_multiplier, float adc_voltage, float ext_voltage) {
  if (adc_voltage <= 0.0f || existing_multiplier <= 0.0f || isnan(adc_voltage) || isnan(existing_multiplier)) {
    return 1.0f;
  }
  return calcMultiplier_externalVoltage((adc_voltage / existing_multiplier), ext_voltage);
}

/// @brief Calculates new LSB from old LSB & multiplier.
/// @param LSB_original i.e. Original/Old/Factory/Default LSB.
/// @param multiplier Multiplier
/// @return New LSB
inline float calcLSB_multiplier(float LSB_original, float multiplier) {
  return LSB_original * multiplier;
}

/// @brief Calculates new LSB from old LSB & voltage divider ohms.
/// @param LSB_original LSB used to generate the incorrect adc_voltage. i.e. Original/Old/Factory/Default LSB.
/// @param R_high Value in ohms of high-side resistor(s).
/// @param R_low Value in ohms of low-side resistor(s).
/// @return New LSB.
inline float calcLSB_vDivider(float LSB_original, float R_high, float R_low) {
  if (isnan(LSB_original)) {
    return 0.0f;
  }
  return calcLSB_multiplier(LSB_original, calcMultiplier_vDivider(R_high, R_low));
}

/// @brief Calculates new LSB from old LSB & external measurement/multimeter.
/// @param LSB_original LSB used to generate the incorrect adc_voltage. i.e. Original/Old/Factory/Default LSB.
/// @param adc_voltage  Incorrect ADC read voltage.
/// @param ext_voltage Known accurate externally measured voltage, such as a calibrated multimeter.
/// @return New LSB.
inline float calcLSB_externalVoltage_adcVoltage(float LSB_original, float adc_voltage, float ext_voltage) {
  if (LSB_original <= 0.0f || adc_voltage <= 0.0f || ext_voltage <= 0.0f || isnan(LSB_original) || isnan(adc_voltage) || isnan(ext_voltage)) {
    return 0.0f;
  }
  return calcLSB_multiplier(LSB_original, calcMultiplier_externalVoltage(adc_voltage, ext_voltage));
}

/// @brief Calculates new LSB based on raw ADC count & external measurement/multimeter. Most accurate & clean, but requires access to raw ADC number.
/// @param raw_ADC_count Raw ADC count at exact time of external measurement.
/// @param ext_voltage Known accurate externally measured voltage, such as a calibrated multimeter.
/// @return New LSB.
inline float calcLSB_rawADC_externalVoltage(uint32_t raw_ADC_count, float ext_voltage) {
  if (raw_ADC_count == 0 || ext_voltage <= 0.0f || isnan(ext_voltage)) {
    return 0.0f;
  }
  return ext_voltage / (float)raw_ADC_count;
}

/// @brief Calculates new ohms of a current-sense shunt resistor, from old ohms & external current measurement/multimeter.
/// @param ohms_original Ohms used to generate the incorrect adc_current. i.e. Original/Old/Factory/Default.
/// @param adc_current The incorrect current reading currently reported by the ADC (in Amps). Can be negative.
/// @param ext_current Known accurate externally measured current (in Amps), such as a calibrated multimeter. Can be negative.
/// @return New shunt resistance in Ohms, or 0.0f if inputs are invalid.
inline float calcOhms_externalCurrent_adcCurrent(float ohms_original, float adc_current, float ext_current) {
  const float abs_adc_current = fabsf(adc_current); // Strip any negative signs,
  const float abs_ext_current = fabsf(ext_current); // ohms doesn't care about direction.

  if (abs_ext_current <= 0.0f || abs_adc_current <= 0.0f || ohms_original <= 0.0f || // Prevent division by zero
      isnan(abs_ext_current) || isnan(abs_adc_current) || isnan(ohms_original)) {    // or invalid NaN/zero inputs
    return 0.0f;
  }
  return ohms_original * (abs_adc_current / abs_ext_current);
}