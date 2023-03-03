/**
 * @file alarm-sound.h
 * @brief
 * 
 */

#ifndef ALARM_SOUND_H
#define ALARM_SOUND_H

#include <cstdint>

#define PIN_FOR_BUZZER 14
#define PIN_FOR_BUZZER_2 15
namespace alarmSound {
enum {
    state_stop,
    state_running,
};

enum {
    seq_button_pressed,
    seq_five_minutes,
    seq_time_has_come,
    seq_time_over,
    seq_num_sequences,
};

enum {
    beep_nof,
    beep_non,

    beep_end = -1,
    beep_rep = -2,
};


void init();
uint8_t current_state();
void tick();
void reset();
void start();
void stop();

void select(int32_t id);

} // namespace alarmSound

#endif // ALARM_SOUND_H
