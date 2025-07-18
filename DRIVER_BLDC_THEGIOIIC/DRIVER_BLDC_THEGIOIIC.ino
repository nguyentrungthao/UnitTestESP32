#include "AnhLABV01HardWare.h"

#define TIMER_DIVIDER (16)                            //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

#define PWM VAN_CO2_PIN
#define LATCH SERVO_PIN
#define PULSES_PER_REVOLUTION 42
#define GEAR_RATIO 4.85f

volatile int pulseCount = 0;
volatile int latchPulseCount = 0;
volatile int speed = 0;
esp_timer_handle_t periodic_timer;

void IRAM_ATTR IRS_gpio(void *) {
  if (digitalRead(LATCH)) return;
  pulseCount++;
}

void IRAM_ATTR IRS_Timer(void *) {
  latchPulseCount = pulseCount;
  pulseCount = 0;
}

int calculate_rpm_int(volatile int &pulse_count, int ppr, int sample_time_ms) {
  int speed = (pulse_count * 60 * 1000) / (ppr * sample_time_ms);
  pulse_count = 0;
  return speed;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(PWM, OUTPUT);
  digitalWrite(PWM, 0);
  pinMode(LATCH, INPUT);
  attachInterruptArg(LATCH, IRS_gpio, NULL, FALLING);

  const esp_timer_create_args_t periodic_timer_args = {
    .callback = &IRS_Timer,
    /* name is optional, but may help identify the timer when debugging */
    .name = "periodic"
  };

  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  esp_timer_start_periodic(periodic_timer, 1000000);  // 1000mS
  /* The timer has been created but is not running yet */

  ledcAttach(PWM, 1000, LEDC_TIMER_12_BIT);
}

void loop() {

  if (Serial.available()) {
    String str = Serial.readString();
    int duty = str.toInt();
    Serial.printf("speed: %d", duty);
    ledcWrite(PWM, duty);

    for (int i = 0; i < 10; i++) {
      Serial.print("speed: ");
      Serial.println(calculate_rpm_int(latchPulseCount, 43, 1000));
      delay(1000);
    }
    ledcWrite(PWM, 0);
  }
  Serial.print("speed: ");
  Serial.println(calculate_rpm_int(latchPulseCount, 43, 1000));
  delay(1000);
  // put your main code here, to run repeatedly:
}

// hardware timer
// static void example_tg_timer_init(int group, int timer, bool auto_reload, int timer_interval_sec)
// {
//     /* Select and initialize basic parameters of the timer */
//     timer_config_t config = {
//         .divider = TIMER_DIVIDER,
//         .counter_dir = TIMER_COUNT_UP,
//         .counter_en = TIMER_PAUSE,
//         .alarm_en = TIMER_ALARM_EN,
//         .auto_reload = auto_reload,
//     }; // default clock source is APB
//     timer_init(group, timer, &config);

//     /* Timer's counter will initially start from value below.
//        Also, if auto_reload is set, this value will be automatically reload on alarm */
//     timer_set_counter_value(group, timer, 0);

//     /* Configure the alarm value and the interrupt on alarm. */
//     timer_set_alarm_value(group, timer, timer_interval_sec * TIMER_SCALE);
//     timer_enable_intr(group, timer);

//     example_timer_info_t *timer_info = calloc(1, sizeof(example_timer_info_t));
//     timer_info->timer_group = group;
//     timer_info->timer_idx = timer;
//     timer_info->auto_reload = auto_reload;
//     timer_info->alarm_interval = timer_interval_sec;
//     timer_isr_callback_add(group, timer, timer_group_isr_callback, timer_info, 0);

//     timer_start(group, timer);
// }
