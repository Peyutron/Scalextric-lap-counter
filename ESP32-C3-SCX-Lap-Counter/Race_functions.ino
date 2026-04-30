// ================
// SYSTEM FUNCTIONS
// ================
void resetRace() 
{
  state = STATE_IDLE;
  currentLap = 0;
  bestLap = 999999.0;
  for (int i = 0; i < totalLaps; i++) 
  {
    lapTimes[i] = 0;
  }
  playSoundReset();
  updateLCDReady();
  Serial.println(F("Carrera reiniciada"));
}

void startRace() 
{
  if (state == STATE_IDLE) 
  {
    state = STATE_COUNTDOWN;
    countdownStart = millis();
    countdownCounter = countdownSeconds;
    playSoundCountdownStart();
    updateLCDCountdown(countdownCounter);
    Serial.println(F("Cuenta atrás iniciada"));
  }
}

void finishLap() 
{
  if (state == STATE_RACING && currentLap < totalLaps) 
  {
    unsigned long lapTime = micros() - startTime;
    lapTimes[currentLap] = lapTime;
    if (currentLap == 0 && firstDetection)
    {
      firstDetection = false;
      return;
    }
    currentLap++;
    
    float lapSeconds = lapTime / 1000000.0;
    if (lapSeconds < bestLap) bestLap = lapSeconds;
    
    playSoundLap();
    updateLCDLapComplete(currentLap, totalLaps, lapSeconds, bestLap);
    
    if (currentLap >= totalLaps) 
    {
      state = STATE_RACE_FINISHED;
      playSoundRaceFinish();
      updateLCDRaceFinished();
      Serial.println(F("Carrera finalizada"));
      firstDetection = true;
    } else 
    {
      // Prepare for next round
      startTime = micros();
    }
  }
}

void ReadButtons()
{
    // Read START button (GPIO9 - BOOT)
  if (digitalRead(START_BUTTON) == LOW) 
  {
    delay(50);  // anti-rebound
    if (digitalRead(START_BUTTON) == LOW) 
    {
      if (state == STATE_IDLE) 
      {
        startRace();
      } 
      else if (state == STATE_RACE_FINISHED) 
      {
        resetRace();
      }
      while (digitalRead(START_BUTTON) == LOW) delay(10);
    }
  }
  
  // Read external RESET button (GPIO10)
  if (RESET_BUTTON >= 0 && digitalRead(RESET_BUTTON) == LOW) 
  {
    delay(50);
    if (digitalRead(RESET_BUTTON) == LOW) {
      resetRace();
      while (digitalRead(RESET_BUTTON) == LOW) delay(10);
    }
  }
}