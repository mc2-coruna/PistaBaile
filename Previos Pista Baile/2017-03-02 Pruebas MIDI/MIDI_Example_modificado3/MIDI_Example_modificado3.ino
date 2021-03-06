// Parámetros del programa

// Instrumento que sonará
  int instrument = 14;
// Duración máxima de las notas
  unsigned long timeNoteOff = 3000;
// Escala de notas
  //byte notas[10] = {67-12,76-12,81-12,84-12,86-12,67,76,81,84,86};
  //byte notas[5] = {67,76,81,84,86};
  byte notas[5] = {67,69,71,73,75};



// Variables auxiliares para cada baldosa (canal)
// Nota que se está tocando. 0 si el canal está apagado
  byte tileNote [16];
// momento de apagado del canal en milisegundos desde la conexión
  unsigned long tileOffTime [16];
// Indicador de que la baldosa fue pisada
  bool tileSteped [16];

//Definiciónde variables e instancias
  #include <SoftwareSerial.h>
  SoftwareSerial VS1053_Serial (2, 3); //Soft TX on 3, we don't use RX in this code

  #define VS_RESET 8  // Tied to VS1053 Reset line
  #define VS_GPIO1 4  // Tied to VS1053 GPIO1



void setup() {
  VS1053_Serial.begin(31250);  // Abre el canal de comunicación con el VS1053
  
  Serial.begin(57600);         // Abre la comunicación serie de la tarjeta

  // Inicializa el VS-1053 en modo Real Time MIDI
  pinMode(VS_RESET, OUTPUT);
  pinMode(VS_GPIO1, OUTPUT);
  digitalWrite (VS_GPIO1,HIGH);

  digitalWrite(VS_RESET, LOW);
  delayMicroseconds(1);  
  digitalWrite(VS_RESET, HIGH);

  delay(100);

  // Inicializa los canales del generador MIDI
  talkMIDI(0xB0, 0x00, 0x00);          // Set default bank GM1

  for(byte i=0; i<16 ; i++) 
  {
    talkMIDI(0xB0 | i, 0x78,0);        // Set channel all sound off 0xB0
    talkMIDI(0xB0 | i, 0x07, 40);      // Set channel volume to near max (127)
    talkMIDI(0xC0 | i, instrument, 0); // Set instrument number. 0xC0 is a 1 data byte command
    tileNote[i] = 0;                   // Establece la nota del canal como 0
  }  
}

unsigned int matrix = 1;
 
void loop() {
  // put your main code here, to run repeatedly:
  
 int desplazamiento;
  
  desplazamiento = random (16);
  
  //matrix = 1<<desplazamiento;
  //Serial.print ("esto es matrix: ");
  //Serial.println (matrix, BIN);
  
  TileOn(matrix);
  
  delay (2000);
  
  //matrix = random (65536);
  //Serial.print ("esto es matrix: ");
  //Serial.println (matrix, BIN);
  
  TileStep(matrix);
  
  delay (2000);

matrix = matrix*2;
  
}


void TileOn(unsigned int matrix){
  
  unsigned long miliSeconds = millis();
  
  for (int i=0; i<16; i++)
  {
    // Comprueba si ya pasó el tiempo necesario para apagar la nota de la baldosa
    if (miliSeconds > tileOffTime [i] + timeNoteOff)
    {
      tileOffTime [i] = miliSeconds;
      noteOff(i, tileNote [i], 60);  // Apaga la nota del canal i
      tileNote[i] = 0;
      tileSteped [i] = false;        // Marca la baldosa para poder pisarla
    }

    // Si el evento incluye esta baldosa lanza una nueva nota
    if (1 & (matrix>>i))
    {
      tileOffTime [i]= miliSeconds;
      tileNote [i] = notas [random (sizeof (notas))];
      tileNote [i] = 71;
      talkMIDI(0xB0 | i, 0x07, 40);  // 0xB0 is channel message, set channel volume bajo (40)
      //noteOn(i, tileNote [i], 120);  // usa el canal i
      noteOn(9, tileNote [i], 120);  // usa el canal i
    }
  }
  
  for (int i=0; i<16; i++)
  {
    boolean prueba;
    prueba = 1 & (matrix>>i);
    Serial.print (prueba,BIN);    
  }
  
  Serial.print ("\t");
  for (int i=0; i<16; i++)
  {
     if (tileNote[i] == 0) Serial.print (" --");
    else if (tileSteped[i] == false) {Serial.print (" "); Serial.print (tileNote[i]);}
    else if (tileSteped[i] == true) {Serial.print ("*"); Serial.print (tileNote[i]);}
    Serial.print ("   ");
  }
  Serial.println();
}



void TileStep(byte matrix1, byte matrix2){
  unsigned int matrix = matrix1*256  + matrix2;
  TileStep (matrix);
}

void TileStep(unsigned int matrix){

  unsigned long miliSeconds = millis();
  
  for (int i=0; i<16; i++)
  {
    // Comprueba si ya pasó el tiempo necesario para apagar la nota de la baldosa
    if (miliSeconds > tileOffTime [i] + timeNoteOff)
    {
      //noteOff (i, tileNote [i], 60);  // usa el canal i
      talkMIDI(0xB0 | i, 0x78,0);        // Set channel all sound off 0xB0
      tileSteped [i] = false;
    }

    else if (!tileSteped [i])
    {
      // Si el evento incluye esta baldosa lanza la segunda nota
      if (matrix & (1<<i))
      {
        tileOffTime [i] = miliSeconds;
        // noteOff(i, tileNote [i], 60);      // usa el canal i
        // TalkMIDI(0xB0 | i, 0x78,0);     //0xB0 is channel message, APAGA EL CANAL i

        tileNote [i] = tileNote [i] + 13; //Elige una nota una octava más alta

        talkMIDI(0xB0 | i, 0x07, 120);     //0xB0 is channel message, set channel volume ALTO (120)
        //noteOn(i, tileNote [i], 127);  // usa el canal i
        noteOn(9, tileNote [i], 127);  // usa el canal i
        tileSteped [i] = true;
        
      //Serial.print ("\tPOTENCIA canal: ");
      //Serial.print (i);
      //Serial.println ();
      }
    }
  }
  
  Serial.print ("\t\t\t");
  for (int i=0; i<16; i++)
  {
    if (tileNote[i] == 0) Serial.print (" --");
    else if (tileSteped[i] == false) {Serial.print (" "); Serial.print (tileNote[i]);}
    else if (tileSteped[i] == true) {Serial.print ("*"); Serial.print (tileNote[i]);}
    Serial.print ("   ");
  }
  Serial.print ("\t");
  
  for (int i=0; i<16; i++)
  {
    boolean prueba;
    prueba = 1 & (matrix>>i);
    Serial.print (prueba,BIN);    
  }
  Serial.println ();    
}



//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  VS1053_Serial.write(cmd);
  VS1053_Serial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    VS1053_Serial.write(data2);
}
