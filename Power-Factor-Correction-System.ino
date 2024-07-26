#include <LiquidCrystal.h>
#include <math.h>
#include <limits.h> // for INT_MAX

const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define right  0
#define left   1
#define up     2
#define down   3
#define select 4
#define none   5

int lcd_key = 0;
int option_counter = 0;

const int Voltage_pin = A1;
const int Current_pin = A2;

float Voltage_RmsV = 0.0;
float Current_RmsV = 0.0;
int Voltage_Gain = 300;
int Current_Gain = 100;
float Offset_Value = 2.5;
int freq = 50;

float Aver_Power = 0.0;
float pf_value = 0.0;
float Required_pf = 0.95;
float volt_zero = 0.0;
float current_zero = 0.0;

float volt_samples;
float current_samples;

const int ledPins[] = {2, 3, 10, 11, 12, 13};
const int n_step = 6;
float kvar_steps[] = {1000, 25000, 20000, 15000, 11000, 8000};
String pf_type[] = {"unity", "lag", "lead", "180phase"}; // Power factor type
float qc = 0.0;
float added_kvar = 0.0;
float q_cal = 0.0;

int lcd_key_reading();
void collect_samples();
float cal_qc(float power, float pf_old, float pf_new);
void led_display(int bestCombinationSize, int bestCombinationIndices[]);
void Best_Combin(float target, float arr[], int size, int& bestCombin_Size, int bestCombin_Indices[]);
int determinePfType();

void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  for (int i = 0; i < n_step; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Power Factor Ctrl");
  delay(2000);
  lcd.clear();
}

void loop() {
  lcd_key = lcd_key_reading();
  collect_samples();
  pf_value = Aver_Power / (Voltage_RmsV * Current_RmsV);
  qc = cal_qc(Aver_Power, pf_value, 0.95);
  int flag = determinePfType(); // Determine power factor type
  int bestCombin_Indices[n_step];
  int bestCombin_Size = 0;
  Best_Combin(qc, kvar_steps, n_step, bestCombin_Size, bestCombin_Indices);
  led_display(bestCombin_Size, bestCombin_Indices);

  if (lcd_key == select) {
    option_counter++;
    delay(100);
    lcd.clear();
    if (option_counter > 2) {
      option_counter = 0;
    }
  }

  switch (option_counter) {
    case 0: {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("V=");
      lcd.print(Voltage_RmsV, 2);
      lcd.setCursor(0, 1);
      lcd.print("I=");
      lcd.print(Current_RmsV, 2);
      lcd.setCursor(9, 0);
      lcd.print("P=");
      lcd.print(Aver_Power, 2);
      break;
    }
    case 1: {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("D_Pf=");
      lcd.print(Required_pf, 2);
      lcd.setCursor(0, 1);
      lcd.print("pf=");
      lcd.print(pf_value, 2);
      lcd.setCursor(9, 1);
      lcd.print(pf_type[flag]);
      delay(2);
      break;
    }
    case 2: {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Qc=");
      lcd.print(qc, 1);
      lcd.setCursor(0, 1);
      lcd.print("kVARadd=");
      float bestSum = 0.0;
      for (int i = 0; i < bestCombin_Size; i++) {
        bestSum += kvar_steps[bestCombin_Indices[i]];
      }
      lcd.print(bestSum, 2);
      break;
    }
  }
  delay(2000);
}

void collect_samples() {
  float cal_power = 0.0, I_sum = 0.0, V_sum = 0.0;
  for (int n_sample = 0; n_sample < 1000; n_sample++) {
    volt_samples = (analogRead(Voltage_pin) * 5.0 / 1023) - Offset_Value;
    current_samples = (analogRead(Current_pin) * 5.0 / 1023) - Offset_Value;
    delayMicroseconds(20);
    cal_power += volt_samples * current_samples;
    I_sum += current_samples * current_samples;
    V_sum += volt_samples * volt_samples;
  }
  Voltage_RmsV = sqrt(V_sum / 1000) * Voltage_Gain;
  Current_RmsV = sqrt(I_sum / 1000) * Current_Gain;
  Aver_Power = fabs((cal_power / 1000) * Voltage_Gain * Current_Gain);
}

float cal_qc(float power, float pf_old, float pf_new) {
  float angle_old = acos(pf_old);
  float angle_new = acos(pf_new);
  return power * (tan(angle_old) - tan(angle_new));
}

void led_display(int bestCombinationSize, int bestCombinationIndices[]) {
  for (int i = 0; i < n_step; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  for (int i = 0; i < bestCombinationSize; i++) {
    digitalWrite(ledPins[bestCombinationIndices[i]], HIGH);
  }
}

void Best_Combin(float target, float arr[], int size, int& bestCombin_Size, int bestCombin_Indices[]) {
  float best_Diff = INT_MAX;
  int currentCombin_Indices[size];
  if (target <= 0) {
    bestCombin_Size = 0;
    return;
  }
  for (int i = 0; i < size; i++) {
    if (arr[i] == target) {
      bestCombin_Size = 1;
      bestCombin_Indices[0] = i;
      return;
    }
  }
  for (int numElements = 1; numElements <= size; numElements++) {
    for (int i = 0; i < numElements; i++) {
      currentCombin_Indices[i] = i;
    }
    while (true) {
      float currentSum = 0.0;
      for (int i = 0; i < numElements; i++) {
        currentSum += arr[currentCombin_Indices[i]];
      }
      float currentDiff = fabs(currentSum - target);
      if (currentDiff < best_Diff || (currentDiff == best_Diff && numElements < bestCombin_Size)) {
        best_Diff = currentDiff;
        bestCombin_Size = numElements;
        for (int i = 0; i < numElements; i++) {
          bestCombin_Indices[i] = currentCombin_Indices[i];
        }
      }
      int pos = numElements - 1;
      while (pos >= 0 && currentCombin_Indices[pos] == size - numElements + pos) {
        pos--;
      }
      if (pos < 0) break;
      currentCombin_Indices[pos]++;
      for (int i = pos + 1; i < numElements; i++) {
        currentCombin_Indices[i] = currentCombin_Indices[i - 1] + 1;
      }
    }
  }
}

int determinePfType() {
  float voltageArray[40];
  float currentArray[40];
  for (int i = 0; i < 40; i++) {
    voltageArray[i] = (analogRead(Voltage_pin) * 5.0 / 1023) - Offset_Value;
    currentArray[i] = (analogRead(Current_pin) * 5.0 / 1023) - Offset_Value;
    if ((voltageArray[i - 1] < 0) && (voltageArray[i] >= 0)) {
      if ((currentArray[i - 1] < 0) && (currentArray[i] < 0)) {
        return 1; // lagging
      } else if ((currentArray[i - 1] > 0) && (currentArray[i] > 0)) {
        return 2; // leading
      } else if ((currentArray[i - 1] < 0) && (currentArray[i] > 0)) {
        return 0; // in phase
      } else if ((currentArray[i - 1] > 0) && (currentArray[i] < 0)) {
        return 3; // phase 180
      }
      delay(20);
    }
  }
  return 0; // default to in phase if no zero crossing is detected
}

int lcd_key_reading() {
  int Button_reading = analogRead(A0);

  if (Button_reading > 950) return none;
  if (Button_reading < 50) return right;
  if (Button_reading < 195) return up;
  if (Button_reading < 380) return down;
  if (Button_reading < 500) return left;
  if (Button_reading < 750) return select;
  return none;
}

