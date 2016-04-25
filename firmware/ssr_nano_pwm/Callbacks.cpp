// ----------------------------------------------------------------------------
// Callbacks.cpp
//
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#include "Callbacks.h"


namespace callbacks
{

SerialReceiver& serial_receiver = controller.getSerialReceiver();

IndexedContainer<PatternInfo,constants::INDEXED_PATTERNS_COUNT_MAX> indexed_patterns;

void startPwmCallback()
{
  int relay = serial_receiver.readInt(1);
  long period = serial_receiver.readLong(2);
  long on_duration = serial_receiver.readLong(3);
  long delay = serial_receiver.readLong(4);
  // Serial << "relay = " << relay << "\n";
  // Serial << "period = " << period << "\n";
  // Serial << "on_duration = " << on_duration << "\n";
  // Serial << "delay = " << delay << "\n";
  EventController::event_controller.addInfinitePwmUsingDelayPeriodOnDuration(closeRelayEventCallback,
                                                                             openRelayEventCallback,
                                                                             delay,
                                                                             period,
                                                                             on_duration,
                                                                             relay,
                                                                             setPwmStatusRunningEventCallback);
}

void startPwmPatternCallback()
{
  if (indexed_patterns.full())
  {
    return;
  }
  int relay = serial_receiver.readInt(1);
  long pwm_period = serial_receiver.readLong(2);
  long pwm_on_duration = serial_receiver.readLong(3);
  long pattern_period = serial_receiver.readLong(4);
  long pattern_on_duration = serial_receiver.readLong(5);
  long delay = serial_receiver.readLong(6);
  // Serial << "relay = " << relay << "\n";
  // Serial << "pwm_period_period = " << pwm_period << "\n";
  // Serial << "pwm_on_duration = " << pwm_on_duration << "\n";
  // Serial << "pattern_period = " << pattern_period << "\n";
  // Serial << "pattern_on_duration = " << pattern_on_duration << "\n";
  // Serial << "delay = " << delay << "\n";
  PatternInfo pattern_info;
  pattern_info.relay = relay;
  pattern_info.pwm_period = pwm_period;
  pattern_info.pwm_on_duration = pwm_on_duration;
  int index = indexed_patterns.add(pattern_info);
  EventController::event_controller.addInfinitePwmUsingDelayPeriodOnDuration(startPwmEventCallback,
                                                                             stopPwmEventCallback,
                                                                             delay,
                                                                             pattern_period,
                                                                             pattern_on_duration,
                                                                             index);
}

void startPwmPatternPowerCallback()
{
  if (indexed_patterns.full())
  {
    return;
  }
  int relay = serial_receiver.readInt(1);
  long pwm_period = serial_receiver.readLong(2);
  long pwm_on_duration = serial_receiver.readLong(3);
  long pattern_period = serial_receiver.readLong(4);
  long pattern_on_duration = serial_receiver.readLong(5);
  long delay = serial_receiver.readLong(6);
  int power = serial_receiver.readInt(7);
  // Serial << "relay = " << relay << "\n";
  // Serial << "pwm_period_period = " << pwm_period << "\n";
  // Serial << "pwm_on_duration = " << pwm_on_duration << "\n";
  // Serial << "pattern_period = " << pattern_period << "\n";
  // Serial << "pattern_on_duration = " << pattern_on_duration << "\n";
  // Serial << "delay = " << delay << "\n";
  // Serial << "power = " << power << "\n";
  uint8_t relay_pin = constants::relay_pins[relay];
  bool relay_pin_is_high_freq = false;
  for (uint8_t r=0; r<constants::HIGH_FREQ_RELAY_COUNT; ++r)
  {
    if (relay_pin == constants::high_freq_relay_pins[r])
    {
      relay_pin_is_high_freq = true;
    }
  }
  // Serial << "relay_pin_is_high_freq = " << relay_pin_is_high_freq << "\n";
  if (!relay_pin_is_high_freq)
  {
    return;
  }
  PatternInfo pattern_info;
  pattern_info.relay = relay;
  pattern_info.pwm_period = pwm_period;
  pattern_info.pwm_on_duration = pwm_on_duration;
  pattern_info.power = power;
  int index = indexed_patterns.add(pattern_info);
  EventController::event_controller.addInfinitePwmUsingDelayPeriodOnDuration(startPwmPowerEventCallback,
                                                                             stopPwmEventCallback,
                                                                             delay,
                                                                             pattern_period,
                                                                             pattern_on_duration,
                                                                             index);
}

void stopAllPwmCallback()
{
  EventController::event_controller.removeAllEvents();
  indexed_patterns.clear();
  controller.openAllRelays();
  controller.setAllPwmStatusStopped();
}

void getRelaysStatusCallback()
{
  Serial << "[";
  constants::RelayStatus relay_status;
  for (uint8_t relay=0; relay<constants::RELAY_COUNT; ++relay)
  {
    if (relay > 0)
    {
      Serial << ",";
    }
    relay_status = controller.getRelayStatus(relay);
    Serial << relay_status;
  }
  Serial << "]\n";
}

void getPwmStatusCallback()
{
  Serial << "[";
  constants::PwmStatus pwm_status;
  for (uint8_t relay=0; relay<constants::RELAY_COUNT; ++relay)
  {
    if (relay > 0)
    {
      Serial << ",";
    }
    pwm_status = controller.getPwmStatus(relay);
    Serial << pwm_status;
  }
  Serial << "]\n";
}

// EventController Callbacks
void closeRelayEventCallback(int relay)
{
  controller.closeRelay(relay);
}

void openRelayEventCallback(int relay)
{
  controller.openRelay(relay);
}

void setPwmStatusRunningEventCallback(int relay)
{
  controller.setPwmStatusRunning(relay);
}

void highFreqPwmRelayEventCallback(int index)
{
  int relay = indexed_patterns[index].relay;
  int power = indexed_patterns[index].power;
  controller.highFreqPwmRelay(relay,power);
}

void startPwmEventCallback(int index)
{
  int relay = indexed_patterns[index].relay;
  long period = indexed_patterns[index].pwm_period;
  long on_duration = indexed_patterns[index].pwm_on_duration;
  controller.setPwmStatusRunning(relay);
  indexed_patterns[index].event_id_pair =
    EventController::event_controller.addInfinitePwmUsingDelayPeriodOnDuration(closeRelayEventCallback,
                                                                               openRelayEventCallback,
                                                                               0,
                                                                               period,
                                                                               on_duration,
                                                                               relay);
}

void startPwmPowerEventCallback(int index)
{
  long period = indexed_patterns[index].pwm_period;
  long on_duration = indexed_patterns[index].pwm_on_duration;
  int relay = indexed_patterns[index].relay;
  controller.setPwmStatusRunning(relay);
  indexed_patterns[index].event_id_pair =
    EventController::event_controller.addInfinitePwmUsingDelayPeriodOnDuration(highFreqPwmRelayEventCallback,
                                                                               openRelayEventCallback,
                                                                               0,
                                                                               period,
                                                                               on_duration,
                                                                               index);
}

void stopPwmEventCallback(int index)
{
  EventController::event_controller.removeEventPair(indexed_patterns[index].event_id_pair);
  int relay = indexed_patterns[index].relay;
  controller.openRelay(relay);
  controller.setPwmStatusStopped(relay);
}

}
