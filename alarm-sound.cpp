/**
 * @file alarm-sound.cpp
 * @brief
 *
 */

#include <stdio.h>
#include "alarm-sound.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#define GPIO_T2 (13)

bool repeating_timer_callback(struct repeating_timer *t)
{
    gpio_put(GPIO_T2, !gpio_get(GPIO_T2));
    alarmSound::tick();

    if (alarmSound::current_state() == alarmSound::state_stop)
    {
        cancel_repeating_timer(t);
    }

    return true;
}

namespace beep
{
namespace
{

const uint16_t count_table[128] = {
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    65535,
    63274,
    59723,
    56371,
    53207,
    50221,
    47402,
    44742,
    42230,
    39860,
    37623,
    35511,
    33518,
    31637,
    29861,
    28185,
    26603,
    25110,
    23701,
    22371,
    21115,
    19930,
    18811,
    17756,
    16759,
    15819,
    14931,
    14093,
    13302,
    12555,
    11850,
    11185,
    10558,
    9965,
    9406,
    8878,
    8380,
    7909,
    7465,
    7046,
    6651,
    6278,
    5925,
    5593,
    5279,
    4983,
    4703,
    4439,
    4190,
    3955,
    3733,
    3523,
    3325,
    3139,
    2963,
    2796,
    2639,
    2491,
    2351,
    2219,
    2095,
    1977,
    1866,
    1762,
    1663,
    1569,
    1481,
    1398,
    1320,
    1246,
    1176,
    1110,
    1047,
    989,
    933,
    881,
    831,
    785,
    741,
    699,
    660,
    623,
    588,
    555,
    524,
    494,
    467,
    440,
    416,
    392,
    370,
    350,
    330,
    311,
    294,
    277,
    262,
    247,
    233,
    220,
    208,
    196,
    185,
    175,
    165,
    156,
};

pwm_config c;

bool beep_on = false;
bool current_note = 0;

int slice = 0;
int ch = 0;
int ch_2 = 0;
} // namespace

void init()
{
    gpio_set_function(PIN_FOR_BUZZER, GPIO_FUNC_PWM);
    gpio_set_function(PIN_FOR_BUZZER_2, GPIO_FUNC_PWM);

    slice = pwm_gpio_to_slice_num(PIN_FOR_BUZZER);
    ch = pwm_gpio_to_channel(PIN_FOR_BUZZER);
    ch_2 = pwm_gpio_to_channel(PIN_FOR_BUZZER_2);
    c = pwm_get_default_config();
    pwm_config_set_clkdiv(&c, 63); // divide by 64
    pwm_config_set_output_polarity(&c, false, false);
    pwm_init(slice, &c, false);
}

void start(uint8_t note)
{
    // workaround
    gpio_set_function(PIN_FOR_BUZZER, GPIO_FUNC_PWM);
    gpio_set_function(PIN_FOR_BUZZER_2, GPIO_FUNC_PWM);

    // printf("beep %d\n", note);
    beep_on = true;
    current_note = note;
    auto idx = note & 0x7f;
    // set pwm freq and duty
    pwm_set_wrap(slice, count_table[idx]);
    // pwm_set_gpio_level(PIN_FOR_BUZZER, count_table[idx] >> 1);
    pwm_set_chan_level(slice, ch, count_table[idx] >> 1);
    pwm_set_chan_level(slice, ch_2, count_table[idx] >> 1);
    // start pwm
    pwm_set_output_polarity(slice, false, true);
    pwm_set_enabled(slice, true);
    // if (pwm_)
}

void stop()
{
    beep_on = false;
    // stop pwm
    pwm_set_output_polarity(slice, false, false);
    pwm_set_enabled(slice, false);
    pwm_set_counter(slice, 0);
}

} // namespace beep

namespace alarmSound
{
namespace
{

uint8_t state;
uint32_t count; // 5ms
uint32_t repeat_count;

uint8_t seq_id;

struct SeqData {
    uint32_t interval;
    int8_t command;
    int8_t note;
};

const SeqData ButtonPressedSeq[] = {
    {0, beep_non, 69},
    {12, beep_nof, 69},
    {3, beep_non, 81},
    {12, beep_nof, 81},
    {3, beep_end, 0}, // end
};

const SeqData FiveMinLeftSeq[] = {
    {0, beep_non, 69},
    {20, beep_nof, 69},
    {5, beep_non, 81},
    {20, beep_nof, 81},
    {5, beep_non, 69},
    {20, beep_nof, 69},
    {5, beep_non, 81},
    {20, beep_nof, 81},

    {105, beep_non, 69},
    {20, beep_nof, 69},
    {5, beep_non, 81},
    {20, beep_nof, 81},
    {5, beep_non, 69},
    {20, beep_nof, 69},
    {5, beep_non, 81},
    {20, beep_nof, 81},

    {5, beep_end, 0}, // end
};

const SeqData TimeHasComeSeq[] = {
    {0, beep_non, 81},
    {20, beep_nof, 81},
    {5, beep_non, 81},
    {20, beep_nof, 81},
    {5, beep_non, 81},
    {20, beep_nof, 81},
    {5, beep_non, 81},
    {20, beep_nof, 81},

    {104, beep_rep, 10}, // repeat

    {0, beep_end, -1}, // end, repeat
};

const SeqData TimeOverSeq[] = {
    {0, beep_non, 81},
    {20, beep_nof, 81},
    {5, beep_non, 85},
    {20, beep_nof, 85},
    {5, beep_non, 88},
    {20, beep_nof, 88},
    {5, beep_non, 93},
    {20, beep_nof, 93},
    {4, beep_end, -1}, // end, repeat
};

const SeqData *const SeqTable[seq_num_sequences] = {
    ButtonPressedSeq, // seq_button_pressed,
    FiveMinLeftSeq,   // seq_five_minutes,
    TimeHasComeSeq,   // seq_time_has_come,
    TimeOverSeq,      // seq_time_over,
};
const uint32_t SeqLengthTable[seq_num_sequences] = {
    sizeof(ButtonPressedSeq) / sizeof(SeqData),
    sizeof(FiveMinLeftSeq) / sizeof(SeqData),
    sizeof(TimeHasComeSeq) / sizeof(SeqData),
    sizeof(TimeOverSeq) / sizeof(SeqData),
};

uint32_t next_seq_data = 0;
uint32_t interval_sum = 0;

const SeqData *selected_seq = SeqTable[0];
uint32_t selected_seq_length = 0;

bool repeat_seq(uint32_t rep_times)
{
    ++repeat_count;
    if (rep_times <= repeat_count)
    {
        repeat_count = 0;
        return false;
    }
    next_seq_data = 0;
    count = 0;
    interval_sum = selected_seq[0].interval;
    return true;
}

void return_to_top()
{
    // jump from timehascome to timeover
    if (selected_seq == TimeHasComeSeq)
    {
        select(seq_time_over);
    }

    next_seq_data = 0;
    count = 0;
    interval_sum = selected_seq[0].interval;
}

struct repeating_timer rpt;
struct alarm_pool *ap;

} // namespace

void init()
{
    beep::init();
    reset();
    ap = alarm_pool_create(1, 1);

    gpio_init(GPIO_T2);
    gpio_set_dir(GPIO_T2, true);
}

uint8_t current_state()
{
    return state;
}
void tick()
{
    // hoge
    // printf("%2d ", count);
    if (interval_sum <= count)
    {
        // read seq data and execute
        switch (selected_seq[next_seq_data].command)
        {
        case beep_nof: // note off
            beep::stop();
            // puts("of");
            break;
        case beep_non: // note on
            beep::start(selected_seq[next_seq_data].note);
            // puts("on");
            break;
        case beep_rep:
            if (repeat_seq(selected_seq[next_seq_data].note))
            {
                // puts("rep");
                return;
            }
            break;
        case beep_end: // end of the sequence
            if (selected_seq[next_seq_data].note < 0)
            { // repeat
                // puts("rtt");
                return_to_top();
                return;
            }
            // puts("stp");
            stop();
            return;
        default:
            break;
        }

        // inc next seq data
        ++next_seq_data;
        interval_sum += selected_seq[next_seq_data].interval;
    }

    ++count;
}

void reset()
{
    count = 0;
    repeat_count = 0;
    state = state_stop;
}

void start()
{
    state = state_running;
    next_seq_data = 0;
    interval_sum = selected_seq[0].interval;
    // auto res = alarm_pool_add_repeating_timer_ms(ap, -5, repeating_timer_callback, nullptr, &rpt);
    cancel_repeating_timer(&rpt);
    auto res = add_repeating_timer_ms(-5, repeating_timer_callback, nullptr, &rpt);
    printf("strt %d\n", res);
}

void stop()
{
    state = state_stop;
    next_seq_data = 0;
    beep::stop();
}

void select(int32_t id)
{
    if (id < 0 || seq_num_sequences <= id)
    {
        return;
    }
    selected_seq = SeqTable[id];
    selected_seq_length = SeqLengthTable[id];
}

} // namespace alarmSound
