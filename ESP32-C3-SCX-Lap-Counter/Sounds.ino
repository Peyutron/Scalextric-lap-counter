// ===================================
//  BUZZER FUNCTIONS (using ESP32Tone)
// ===================================
void playTone(int frequency, int duration)
{
  tone(BUZZER_PIN, frequency, duration);
  delay(duration + 10);
  noTone(BUZZER_PIN);
}

void playSoundReset()
{
  playTone(1000, 100);
  delay(50);
  playTone(800, 100);
}

void playSoundCountdownStart() 
{
  playTone(1500, 200);
}

void playSoundCountdownTick() 
{
  playTone(2000, 50);
}

void playSoundRaceStart()
{
  playTone(800, 100);
  delay(50);
  playTone(1000, 100);
  delay(50);
  playTone(1200, 200);
}

void playSoundLap()
{
  playTone(1200, 80);
  delay(30);
  playTone(1500, 80);
}

void playSoundRaceFinish()
{
  for(int i = 0; i < 3; i++)
  {
    playTone(800, 150);
    delay(50);
    playTone(1000, 150);
    delay(50);
  }
  playTone(1200, 500);
}