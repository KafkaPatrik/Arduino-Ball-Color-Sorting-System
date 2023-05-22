#ifndef COLOR_DETECTION_H
#define COLOR_DETECTION_H
#define S0 12
#define S1 13
#define S2 10
#define S3 11
#define FREQ_OUT 9
#define EnableDebuggingLogsColors false
const byte ColorUndefined = 0, ColorRed = 1, ColorGreen = 2, ColorBlue = 3, ColorYellow = 4, ColorWhite = 5;
const byte MAX_COLORS = 6;
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;
int colorBlueDetect = 0;
int colorRedDetect = 0;
int colorGreenDetect = 0;
int freq_scan_delay = 0;
byte BallColor = ColorUndefined, BallColorPrev = ColorUndefined, BallColorOut = ColorUndefined;
int sample_buffer [5], color_app[MAX_COLORS];
int sample_mean_crr = 0, sample_mean_prev = 0;
byte i;
bool ball_detected = false;
//circular buffer sampling
const byte window_sampling_size = 5;
int R_buffer[window_sampling_size];
int G_buffer[window_sampling_size];
int B_buffer[window_sampling_size];
int buff_index = 0;
float R_avg_prev = 0.0, G_avg_prev = 0.0, B_avg_prev = 0.0;
const float movement_detect_tresh = 40.0;
const byte colorDetectOffset=50;


void init_buffers() {
  for (i = 0; i < window_sampling_size; i++) {
    R_buffer[i] = 0;
    G_buffer[i] = 0;
    B_buffer[i] = 0;
  }
}
void setup_color_detect(int freq_delay) {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(FREQ_OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  freq_scan_delay = freq_delay;
  init_buffers();
}
void read_frequency()
{
  // Setting up the photodiodes for each color and reading the frequency
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  redFrequency = pulseIn(FREQ_OUT, LOW);
  delay(freq_scan_delay);
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  greenFrequency = pulseIn(FREQ_OUT, LOW);
  delay(freq_scan_delay);
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  blueFrequency = pulseIn(FREQ_OUT, LOW);

#if EnableDebuggingLogsColors == true
  Serial.println("Color detected without filtering function:");
  Serial.print("R = ");
  Serial.print(redFrequency);
  Serial.print(" G = ");
  Serial.print(greenFrequency);
  Serial.print(" B = ");
  Serial.println(blueFrequency);
#endif

  delay(freq_scan_delay);
}
void color_detect_routine() {
  byte color_max_cnt = 0, color_max_index = 0;
  int R_sum_window = 0, G_sum_window = 0, B_sum_window = 0;
  float R_avg, G_avg, B_avg;
  float R_diff, G_diff, B_diff;
  bool sampling_active = false, output_color_valid = false;
  byte sample_count = 0, valid_cnt = 0;
  ball_detected = false;
  if (ball_detected != true)
    Serial.println("Ball detection routine active!");
  while (ball_detected != true) {
    read_frequency();
    colorBlueDetect = map(blueFrequency, 10, 600, 1023, 0);
    colorRedDetect = map(redFrequency, 10, 600, 1023, 0);
    colorGreenDetect = map(greenFrequency, 10, 600, 1023, 0);
    //buffer operation start
    R_buffer[buff_index] = colorRedDetect;
    G_buffer[buff_index] = colorGreenDetect;
    B_buffer[buff_index] = colorBlueDetect;

    buff_index = (buff_index + 1) % window_sampling_size;

    R_sum_window = 0;
    G_sum_window = 0;
    B_sum_window = 0;

    for (i = 0; i < window_sampling_size; i++) {
      R_sum_window += R_buffer[i];
      G_sum_window += G_buffer[i];
      B_sum_window += B_buffer[i];
    }

    R_avg = (float)R_sum_window / window_sampling_size;
    G_avg = (float)G_sum_window / window_sampling_size;
    B_avg = (float)B_sum_window / window_sampling_size;

    R_diff = abs(R_avg - R_avg_prev);
    G_diff = abs(G_avg - G_avg_prev);
    B_diff = abs(B_avg - B_avg_prev);

#if EnableDebuggingLogsColors == true
    Serial.println("Colors avg, avg_prev difference:");
    Serial.print("R_diff = ");
    Serial.print(R_diff);
    Serial.print(" G_diff = ");
    Serial.print(G_diff);
    Serial.print(" B_diff = ");
    Serial.println(B_diff);
    Serial.print("R_avg = ");
    Serial.print(R_avg);
    Serial.print(" G_avg = ");
    Serial.print(G_avg);
    Serial.print(" B_avg = ");
    Serial.println(B_avg);
    Serial.print("R_avg_prev = ");
    Serial.print(R_avg_prev);
    Serial.print(" G_avg_prev = ");
    Serial.print(G_avg_prev);
    Serial.print(" B_avg_prev = ");
    Serial.println(B_avg_prev);
    Serial.print(" Threshhold = ");
    Serial.println(movement_detect_tresh);
    Serial.println();

#endif
#if EnableDebuggingLogsColors == true
    Serial.println("Color detected with filter function:");
    Serial.print("R = ");
    Serial.print(colorRedDetect);
    Serial.print(" G = ");
    Serial.print(colorGreenDetect);
    Serial.print(" B = ");
    Serial.println(colorBlueDetect);
#endif
    sample_mean_crr = (colorGreenDetect + colorRedDetect + colorBlueDetect) / 3.0;
#if EnableDebuggingLogsColors == true
    Serial.print(sample_mean_crr);
    Serial.print(" > Sample prevf-: ");
    Serial.print(sample_mean_prev - 50);
    Serial.print(" Sample crr: ");
    Serial.print(sample_mean_crr);
    Serial.print(" < Sample prevf+: ");
    Serial.println(sample_mean_prev + 50);
#endif

    if (output_color_valid == true) {
      // Serial.print("Sampling_active_flag: ");
      //  Serial.println(sampling_active);

      // if ( (sample_mean_crr > sample_mean_prev - 100 ) && (sample_mean_crr < sample_mean_prev + 100)) // idle -- 200 prev
      if ((R_diff < movement_detect_tresh && G_diff < movement_detect_tresh && B_diff < movement_detect_tresh) && sampling_active == false)
      {
#if EnableDebuggingLogsColors == true
        Serial.println("No movement detected");
#endif
      } else // movement detected
      {
#if EnableDebuggingLogsColors == true
        Serial.println("Movement detected, treshold condition active");
        if (sampling_active == false)
          Serial.println("Initiating sampling routine.");
        Serial.print("White condition: ");
        Serial.println((colorRedDetect + colorBlueDetect + colorGreenDetect) / 3);
#endif
        BallColor = ColorUndefined;
        if (colorRedDetect > colorBlueDetect + colorDetectOffset && colorGreenDetect > colorBlueDetect + colorDetectOffset && (abs(colorRedDetect - colorGreenDetect) <= 30)) //20 previous
          BallColor = ColorYellow;
        else
        {
          if (colorRedDetect > colorGreenDetect+colorDetectOffset && colorRedDetect > colorBlueDetect+colorDetectOffset)
            BallColor = ColorRed;
          else if ( (float)( colorRedDetect + colorGreenDetect + colorBlueDetect) / 3 > 920) //920 threshold for white ball, we use it as undefined in tests for throwing
            BallColor = ColorWhite;
          else if (colorGreenDetect > colorRedDetect && colorGreenDetect > colorBlueDetect)
            BallColor = ColorGreen;
          else if (colorBlueDetect > colorRedDetect && colorBlueDetect > colorGreenDetect)
            BallColor = ColorBlue;
          else
            BallColor = ColorUndefined;
        }
#if EnableDebuggingLogsColors == true
        Serial.print("Detected ball color is: ");
        Serial.println(BallColor);
#endif
        //if(BallColor!=BallColorPrev && sampling_active == false){
        if (sampling_active == false) {
          Serial.println("Sampling sequence initiated");
          sampling_active = true;
          sample_count = 0;
          color_max_cnt = 0;
          color_max_index = 0;
        }
        if (sampling_active == true)
        {
          sample_count++;
          if ( sample_count < 12)//15 is good also
          {
            color_app[BallColor]++;
          }
          else
          {
            for (byte colors_index = 0; colors_index < MAX_COLORS; colors_index++)
              if (color_app[colors_index] > color_max_cnt)
              {
                color_max_cnt = color_app[colors_index];
                color_max_index = colors_index;
              }
#if EnableDebuggingLogsColors == false
            Serial.print("MaxC: ");
            Serial.print(color_max_index);
            Serial.print(" Color counter for sampling cycle: ");
#endif

            for (i = 0; i < MAX_COLORS; i++) {
#if EnableDebuggingLogsColors == false
              Serial.print(color_app[i]);
              Serial.print(" ");
#endif
              color_app[i] = 0;
            }
#if EnableDebuggingLogsColors == false
            Serial.println();
#endif

            BallColorOut = color_max_index;
            ball_detected = true;

            sampling_active = false;
            Serial.println("Sampling sequence completed");
#if EnableDebuggingLogsColors == false
            Serial.print("Out ball color is: ");
            Serial.println(BallColorOut);
#endif
         }
        }
        BallColorPrev = BallColor;
      }
    }
    sample_mean_prev = sample_mean_crr;
    R_avg_prev = R_avg;
    G_avg_prev = G_avg;
    B_avg_prev = B_avg;

    if (valid_cnt < 7) // validate color sampling routine after 7 reading cycles to stabilise the output
      valid_cnt++;
    else
      output_color_valid = true;
  }


  Serial.println("Ball detected!");
  // delay(100);
}
#endif
