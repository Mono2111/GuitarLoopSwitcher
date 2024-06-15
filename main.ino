#include <EEPROM.h>

//LastUpdate:04/2023

//--Preset-------------0---1---2---3---4---5---6---7---8---9----11---12---13---14---15
int speicherOrt [] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160};
int tasterPin[] = {2, 3, 4, 5, 6};
int ledGruenPin[] = {7, 9, 11, 13, 15, 17, 19, 21};
int ledRotPin[] = {8, 10, 12, 14, 16, 18, 20, 22};
int relaisPin[] = {24, 25, 26, 27, 28, 29, 30};
int tasterMessung[] = {LOW, LOW, LOW, LOW, LOW};
int tasterMessungAlt[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
int mode = 0; // 0 = Song1-4, 1 = Song5-8, 2 = Looper_mode, 3 = programmingEins 4= programmingZwei
long modeTimer = 0;
long tasterTimer = 0;
int tasterWechsel = 0;
int tasterDurchlaufen = 0;
int loops[] = {0, 0, 0, 0, 0, 0, 0};
int j = 0;
int preset = 0;
long ledProgrammingTimer = 0;
int presetMatrix[16][10] =
{ //0vol-1bos-2kot-3subo-4com-5del-6rev-7mid-8Nr
  //1vol-2bos-3kot-4subo-5com-6del-7rev-8mid-9Nr
  {LOW, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x64,  0},      //0   0x64 = bank 50A
  {LOW, LOW, LOW, LOW, LOW, HIGH, HIGH, 0x66,  1},      //2 
  {LOW, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x68, 2},     //4  
  {LOW, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x6A,  3},     //6 
  {LOW, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x65,  4},    //1  
  {LOW, LOW, LOW, LOW, LOW, HIGH, HIGH, 0x67,  5},      //3 
  {LOW, LOW, HIGH, LOW, LOW, LOW, HIGH, 0x69,  6},      //5 
  {LOW, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x6B,  7},     //7 
  {LOW, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x6C,  8},     //8 
  {LOW, LOW, LOW, LOW, LOW, HIGH, HIGH, 0x6E,  9},       //9  20 
  {HIGH, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x70,  10},  //12 
  {HIGH, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x72,  11},  //14
  {LOW, HIGH, HIGH, LOW, LOW, HIGH, HIGH, 0x6D,  12},  //9 
  {LOW, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x6F,  13},    //11 
  {HIGH, LOW, HIGH, LOW, LOW, HIGH, HIGH, 0x71,  14},  //13
  {LOW, LOW, LOW, LOW, LOW, LOW, LOW, 0x73,  15}             //15  0x73 = bank 57A
};//0vol-1bos-2kot--3subo-4com-5del-6rev-midi
byte programChange = 0xC0;

void setup()
{
  delay(500); //delay fuer startup
  for (int i = 0; i < 31; i++)
  {
    pinMode(i, INPUT);
    digitalWrite(i, LOW);
  }
  pinMode(2, INPUT_PULLUP); //Loop Taster 1 bzw 2
  pinMode(3, INPUT_PULLUP); //Loop Taster 3 bzw 2
  pinMode(4, INPUT_PULLUP); //Loop Taster 4 bzw 5
  pinMode(5, INPUT_PULLUP); //Loop Taster 6 bzw 5
  pinMode(6, INPUT_PULLUP); //Loop Taster Mode bzw 7
  pinMode(7, OUTPUT); //LED Gruen Preset 1
  pinMode(8, OUTPUT); //LED Rot Loop 1
  pinMode(9, OUTPUT); //LED Gruen Preset 2
  pinMode(10, OUTPUT); //LED Rot Loop 2
  pinMode(11, OUTPUT); //LED Gruen Preset 3
  pinMode(12, OUTPUT); //LED Rot Loop 3
  pinMode(13, OUTPUT); //LED Gruen Preset 4
  pinMode(14, OUTPUT); //LED Rot Loop 4
  pinMode(15, OUTPUT); //LED Gruen Preset 5
  pinMode(16, OUTPUT); //LED Rot Loop 5
  pinMode(17, OUTPUT); //LED Gruen Preset 6
  pinMode(18, OUTPUT); //LED Rot Loop 6
  pinMode(19, OUTPUT); //LED Gruen Preset 7
  pinMode(20, OUTPUT); //LED Rot Loop 7
  pinMode(21, OUTPUT); //LED Gruen Modus Preset
  pinMode(22, OUTPUT); //LED Rot Modus Loop
  pinMode(23, OUTPUT); //LED Amber Midi Mode
  pinMode(24, OUTPUT); //Relais Loop 1
  pinMode(25, OUTPUT); //Relais Loop 2
  pinMode(26, OUTPUT); //Relais Loop 3
  pinMode(27, OUTPUT); //Relais Loop 4
  pinMode(28, OUTPUT); //Relais Loop 5
  pinMode(29, OUTPUT); //Relais Loop 6
  pinMode(30, OUTPUT); //Relais Loop 7

  // --- Presets aus EEProm laden ---

  for (int o = 0; o < 16; o++)
  {
    for (int p = 0; p < 7; p++)
    {
      presetMatrix[o][p] = EEPROM.read(speicherOrt[o] + p);
    }
  }

  // --- Loading-Lightshow ---
  for (int i = 8; i <= 22; i += 2)
  {
    digitalWrite(i, HIGH);
    delay(125);
  }
  for (int i = 7; i <= 23; i += 2)
  {
    digitalWrite(i + 1, LOW);
    digitalWrite(i, HIGH);
    delay(125);
  }
  for (int i = 7; i <= 23; i++)
  {
    digitalWrite(i, LOW);
  }

  Serial1.begin(31250);

  //Startpreset
  for (int p = 0; p < 7; p++)
  {
    digitalWrite(relaisPin[p], presetMatrix[0][p]);
  }
  digitalWrite(ledRotPin[0], HIGH);
  Serial1.write(programChange); //prog Change
  Serial1.write(presetMatrix[0][7]);
  preset = 0;
}

void loop()
{
  modeChange();

  if (mode == 0 || mode == 1)
  {
    presetChange();
  }
  else if (mode == 2)
  {
    loopChange();
    ledUpdate();
  }
  else if (mode == 3)
  {
    programmingEins();
    ledUpdate();
  }
  else
  {
    programmingZwei();
    ledUpdate();
  }
}

void modeChange()
{
  tasterMessung[4] = digitalRead(tasterPin[4]);
  if ((tasterMessung[4] == LOW) && (tasterMessungAlt[4] == HIGH))
  {
    if (modeTimer == 0)
    {
      modeTimer = millis();
    }
    tasterMessungAlt[4] = tasterMessung[4];
  }
  if ((tasterMessung[4] == HIGH) && (tasterMessungAlt[4] == LOW))
  {
    if ((mode == 0)  && (millis() - modeTimer < 1000) && (millis() - modeTimer > 60))
    {
      mode = 1;
      for (int p = 0; p < 7; p++)
      {
        digitalWrite(relaisPin[p], presetMatrix[8][p]);
        digitalWrite(ledGruenPin[p], LOW);
        digitalWrite(ledRotPin[p], LOW);
      }
      digitalWrite(ledRotPin[0], HIGH);
      Serial1.write(programChange); //prog Change
      Serial1.write(presetMatrix[8][7]);
      preset = presetMatrix[8][8];
    }
    else if ((mode == 1 || mode == 2 || mode == 3 || mode == 4)  && (millis() - modeTimer < 1000) && (millis() - modeTimer > 60))
    {
      mode = 0;
      for (int p = 0; p < 7; p++)
      {
        digitalWrite(relaisPin[p], presetMatrix[0][p]);
        digitalWrite(ledGruenPin[p], LOW);
        digitalWrite(ledRotPin[p], LOW);
      }
      digitalWrite(ledRotPin[0], HIGH);
      Serial1.write(programChange); //prog Change
      Serial1.write(presetMatrix[0][7]);
      preset = presetMatrix[0][8];

      for (int k = 0; k < 16; k++)
      {
        for (int j = 0; j < 9; j ++)
        {
          Serial.print(presetMatrix[k][j]); Serial.print("\t");
        }
        Serial.print("\n");
      }

    }
    else if ((millis() - modeTimer > 1000) && (millis() - modeTimer > 60) && (millis() - modeTimer < 7000))
    {
      mode = 2;
      for (int i = 0; i < 7; i++)
      {
        digitalWrite(relaisPin[i], loops[i]);
      }
    }
    else if ((millis() - modeTimer > 7000) && (millis() - modeTimer > 60) && (millis() - modeTimer < 10000))
    {
      mode = 3;
      for (int i = 0; i < 7; i++)
      {
        digitalWrite(relaisPin[i], loops[i]);
      }
    }
    else if ((millis() - modeTimer > 10000) && (millis() - modeTimer > 60))
    {
      mode = 4;
      for (int i = 0; i < 7; i++)
      {
        digitalWrite(relaisPin[i], loops[i]);
      }
    }
    tasterMessungAlt[4] = tasterMessung[4];
    modeTimer = 0;
  }
}

void presetChange()
{
  if (mode == 0)
  {
    j = 0;
    digitalWrite(23, LOW); // Orange Led
    digitalWrite(21, LOW); //Gruen LED
    digitalWrite(22, HIGH); // Rot LED
  }
  else
  {
    j = 8;
    digitalWrite(23, LOW); // Orange Led
    digitalWrite(21, HIGH); //Gruen LED
    digitalWrite(22, LOW); // Rot LED
  }
  for (int i = 0; i < 4; i++)
  {
    tasterMessung[i] = digitalRead(tasterPin[i]); //push Detektierung i
    if ((tasterMessung[i] == LOW) && ((tasterMessungAlt[i] == HIGH) || (tasterTimer != 0)) && (tasterDurchlaufen == 0))
    {
      if (tasterTimer == 0)
      {
        tasterTimer = millis();
      }
      if ((millis() - tasterTimer > 75))
      {
        if ((preset == i) || (preset == i + j)) //erstes Songpreset aktiv  && tasterWechsel == 0
        {
          for (int p = 0; p < 7; p++)
          {
            digitalWrite(relaisPin[p], presetMatrix[i + j + 4][p]);
            digitalWrite(ledGruenPin[p], LOW);
            digitalWrite(ledRotPin[p], LOW);
          }
          digitalWrite(ledGruenPin[i], HIGH);
          Serial1.write(programChange);
          Serial1.write(presetMatrix[i + j + 4][7]);
          preset = presetMatrix[i + j + 4][8];
        }
        else //zweites Songpreset aktiv
        {
          for (int p = 0; p < 7; p++)
          {
            digitalWrite(relaisPin[p], presetMatrix[i + j][p]);
            digitalWrite(ledGruenPin[p], LOW);
            digitalWrite(ledRotPin[p], LOW);
          }
          digitalWrite(ledRotPin[i], HIGH);
          Serial1.write(programChange);
          Serial1.write(presetMatrix[i + j][7]);
          preset = presetMatrix[i + j][8];
        }
        tasterDurchlaufen = 1;
      }
      tasterMessungAlt[i] = tasterMessung[i];
    }
    if ((tasterMessung[i] == HIGH) && (tasterMessungAlt[i] == LOW)) //release Detektierung i
    {
      tasterMessungAlt[i] = tasterMessung[i];
      tasterTimer = 0;
      tasterDurchlaufen = 0;
    }
  }
}

void loopChange()
{
  for (int i = 0; i < 4; i++)
  {
    tasterMessung[i] = digitalRead(tasterPin[i]);  //push Detektierung i
    if ((tasterMessung[i] == LOW) && ((tasterMessungAlt[i] == HIGH) || (tasterTimer != 0)) && (tasterDurchlaufen == 0))
    {
      if (tasterTimer == 0)
      {
        tasterTimer = millis();
      }
      tasterMessung[i + 1] = digitalRead(tasterPin[i + 1]); //push Detektierung i+1
      if ((millis() - tasterTimer > 75) && (tasterMessung[i + 1] == LOW))
      {
        if (loops[i + 4] == 1)
        {
          digitalWrite(relaisPin[i + 4], LOW);
          loops[i + 4] = 0;
        }
        else
        {
          digitalWrite(relaisPin[i + 4], HIGH);
          loops[i + 4] = 1;
        }
        tasterDurchlaufen = 1;
      }
      else if ((millis() - tasterTimer > 75))
      {
        if (loops[i] == 1)
        {
          digitalWrite(relaisPin[i], LOW);
          loops[i] = 0;
        }
        else
        {
          digitalWrite(relaisPin[i], HIGH);
          loops[i] = 1;
        }
        tasterDurchlaufen = 1;
      }
      tasterMessungAlt[i] = tasterMessung[i];
      tasterMessungAlt[i + 1] = tasterMessung[i + 1];
    }
    if ((tasterMessung[i] == HIGH) && (tasterMessungAlt[i] == LOW)) //release Detektierung i
    {
      tasterMessungAlt[i] = tasterMessung[i];
      tasterTimer = 0;
      tasterDurchlaufen = 0;
    }
    if ((tasterMessung[i + 1] == HIGH) && (tasterMessungAlt[i + 1] == LOW))  //release Detektierung i+1
    {
      tasterMessungAlt[i + 1] = tasterMessung[i + 1];
      tasterDurchlaufen = 0;
    }
  }
}

void ledUpdate() //nur fuer loops
{
  if (mode == 2)
  {
    digitalWrite(23, HIGH); //Loop Mode
    digitalWrite(22, LOW); //Rot LED
    digitalWrite(21, LOW); // Grun LED
  }
  else if (mode == 3)
  {
    digitalWrite(22, HIGH); // Rot LED
    digitalWrite(21, LOW); // Grun LED
    if ((millis() - ledProgrammingTimer > 500) && (millis() - ledProgrammingTimer <= 1000))
    {
      digitalWrite(23, LOW); //blicking LED
    }
    else if (millis() - ledProgrammingTimer > 1000)
    {
      digitalWrite(23, HIGH); //blicking LED
      ledProgrammingTimer = millis();
    }
  }
  else
  {
    digitalWrite(22, LOW); // Rot LED
    digitalWrite(21, HIGH); // Grun LED
    if ((millis() - ledProgrammingTimer > 500) && (millis() - ledProgrammingTimer <= 1000))
    {
      digitalWrite(23, LOW); //blicking LED
    }
    else if (millis() - ledProgrammingTimer > 1000)
    {
      digitalWrite(23, HIGH); //blicking LED
      ledProgrammingTimer = millis();
    }
  }
  for (int o = 0; o < 7; o++)
  {
    if (loops[o] == 1)
    {
      digitalWrite(ledGruenPin[o], LOW);
      digitalWrite(ledRotPin[o], HIGH);
    }
    else
    {
      digitalWrite(ledGruenPin[o], LOW);
      digitalWrite(ledRotPin[o], LOW);
    }
  }
}

void programmingEins()
{
  for (int i = 0; i < 4; i++)
  {
    tasterMessung[i] = digitalRead(tasterPin[i]);  //push Detektierung i
    if ((tasterMessung[i] == LOW) && ((tasterMessungAlt[i] == HIGH) || (tasterTimer != 0)) && (tasterDurchlaufen == 0))
    {
      if (tasterTimer == 0)
      {
        tasterTimer = millis();
      }
      tasterMessungAlt[i] = tasterMessung[i];
      tasterMessungAlt[i + 1] = tasterMessung[i + 1];
    }
    if ((tasterMessung[i] == HIGH) && (tasterMessungAlt[i] == LOW)) //release Detektierung i
    {
      tasterMessung[i + 1] = digitalRead(tasterPin[i + 1]); //push Detektierung i+1
      if ((millis() - tasterTimer > 75) && (tasterMessung[i + 1] == LOW) && (millis() - tasterTimer < 2000))
      {
        if (loops[i + 4] == 1)
        {
          digitalWrite(relaisPin[i + 4], LOW);
          loops[i + 4] = 0;
        }
        else
        {
          digitalWrite(relaisPin[i + 4], HIGH);
          loops[i + 4] = 1;
        }
        tasterDurchlaufen = 1;
      }
      else if ((millis() - tasterTimer > 75) && (millis() - tasterTimer < 2000))
      {
        if (loops[i] == 1)
        {
          digitalWrite(relaisPin[i], LOW);
          loops[i] = 0;
        }
        else
        {
          digitalWrite(relaisPin[i], HIGH);
          loops[i] = 1;
        }
        tasterDurchlaufen = 1;
      }
      else if ((millis() - tasterTimer > 2000) && (millis() - tasterTimer < 4000))
      {
        for (int u = 0; u < 7; u++)
        {
          presetMatrix[i][u] = loops[u];
          EEPROM.write(speicherOrt[i] + u, loops[u]);
        }
        for (int o = 0; o < 7; o++)
        {
          digitalWrite(ledGruenPin[o], LOW);
          digitalWrite(ledRotPin[o], HIGH);
        }
        delay(500);
        tasterDurchlaufen = 1;
      }
      else if ((millis() - tasterTimer > 4000) && (millis() - tasterTimer < 6000))
      {
        for (int u = 0; u < 7; u++)
        {
          presetMatrix[i + 4][u] = loops[u];
          EEPROM.write(speicherOrt[i + 4] + u, loops[u]);
        }
        for (int o = 0; o < 7; o++)
        {
          digitalWrite(ledGruenPin[o], HIGH);
          digitalWrite(ledRotPin[o], LOW);
        }
        delay(500);
        tasterDurchlaufen = 1;
      }
      tasterMessungAlt[i] = tasterMessung[i];
      tasterTimer = 0;
      tasterDurchlaufen = 0;
    }
    if ((tasterMessung[i + 1] == HIGH) && (tasterMessungAlt[i + 1] == LOW))  //release Detektierung i+1
    {
      tasterMessungAlt[i + 1] = tasterMessung[i + 1];
      tasterDurchlaufen = 0;
    }
  }
}

void programmingZwei()
{
  for (int i = 0; i < 4; i++)
  {
    tasterMessung[i] = digitalRead(tasterPin[i]);  //push Detektierung i
    if ((tasterMessung[i] == LOW) && ((tasterMessungAlt[i] == HIGH) || (tasterTimer != 0)) && (tasterDurchlaufen == 0))
    {
      if (tasterTimer == 0)
      {
        tasterTimer = millis();
      }
      tasterMessungAlt[i] = tasterMessung[i];
      tasterMessungAlt[i + 1] = tasterMessung[i + 1];
    }
    if ((tasterMessung[i] == HIGH) && (tasterMessungAlt[i] == LOW)) //release Detektierung i
    {
      tasterMessung[i + 1] = digitalRead(tasterPin[i + 1]); //push Detektierung i+1
      if ((millis() - tasterTimer > 75) && (tasterMessung[i + 1] == LOW) && (millis() - tasterTimer < 2000))
      {
        if (loops[i + 4] == 1)
        {
          digitalWrite(relaisPin[i + 4], LOW);
          loops[i + 4] = 0;
        }
        else
        {
          digitalWrite(relaisPin[i + 4], HIGH);
          loops[i + 4] = 1;
        }
        tasterDurchlaufen = 1;
      }
      else if ((millis() - tasterTimer > 75) && (millis() - tasterTimer < 2000))
      {
        if (loops[i] == 1)
        {
          digitalWrite(relaisPin[i], LOW);
          loops[i] = 0;
        }
        else
        {
          digitalWrite(relaisPin[i], HIGH);
          loops[i] = 1;
        }
        tasterDurchlaufen = 1;
      }
      else if ((millis() - tasterTimer > 2000) && (millis() - tasterTimer < 4000))
      {
        for (int u = 0; u < 7; u++)
        {
          presetMatrix[i + 8][u] = loops[u];
          EEPROM.write(speicherOrt[i + 8] + u, loops[u]);
        }
        for (int o = 0; o < 7; o++)
        {
          digitalWrite(ledGruenPin[o], LOW);
          digitalWrite(ledRotPin[o], HIGH);
        }
        delay(500);
        tasterDurchlaufen = 1;
      }
      else if ((millis() - tasterTimer > 4000) && (millis() - tasterTimer < 6000))
      {
        for (int u = 0; u < 7; u++)
        {
          presetMatrix[i + 12][u] = loops[u];
          EEPROM.write(speicherOrt[i + 12] + u, loops[u]);
        }
        for (int o = 0; o < 7; o++)
        {
          digitalWrite(ledGruenPin[o], HIGH);
          digitalWrite(ledRotPin[o], LOW);
        }
        delay(500);
        tasterDurchlaufen = 1;
      }
      tasterMessungAlt[i] = tasterMessung[i];
      tasterTimer = 0;
      tasterDurchlaufen = 0;
    }
    if ((tasterMessung[i + 1] == HIGH) && (tasterMessungAlt[i + 1] == LOW))  //release Detektierung i+1
    {
      tasterMessungAlt[i + 1] = tasterMessung[i + 1];
      tasterDurchlaufen = 0;
    }
  }
}
