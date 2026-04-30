// ====================
// LCD SCREEN FUNCTIONS
// ====================
void updateLCDReady() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Listo!");
  lcd.setCursor(0, 1);
  lcd.print("Start: Boton/Web");
}

void updateLCDCountdown(int seconds) 
{
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("PREPARADOS");
  lcd.setCursor(6, 1);
  lcd.print(seconds);
  lcd.print("   ");
}

void updateLCDRacing() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CARRERA!");
  lcd.setCursor(0, 1);
  lcd.print("Vuelta: ");
  lcd.print(currentLap + 1);
  lcd.print("/");
  lcd.print(totalLaps);
}

void updateLCDRacingTime(unsigned long elapsedMs) 
{
  float seconds = elapsedMs / 1000.0;
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(seconds, 3);
  lcd.print("s    ");
}

void updateLCDLapComplete(int lap, int total, float lapTime, float best) 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Vuelta "));
  lcd.print(lap);
  lcd.print(F("/"));
  lcd.print(total);
  lcd.setCursor(0, 1);
  lcd.print(lapTime, 3);
  lcd.print("s");
  if (lapTime == best && lap > 1) 
  {
    lcd.print(" REC!");
  }
}

void updateLCDRaceFinished() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FIN CARRERA!");
  lcd.setCursor(0, 1);
  lcd.print("Best: ");
  lcd.print(bestLap, 3);
  lcd.print("s");
  delay(1500);
  
  // Show lap summary
  for (int i = 0; i < totalLaps && i < 4; i++)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("V");
    lcd.print(i+1);
    lcd.print(": ");
    lcd.print(lapTimes[i] / 1000000.0, 3);
    lcd.print("s");
    if (i == totalLaps-1)
    {
      lcd.setCursor(0, 1);
      lcd.print("Best: ");
      lcd.print(bestLap, 3);
      lcd.print("s");
    }
    delay(1500);
  }
  updateLCDReady();
}