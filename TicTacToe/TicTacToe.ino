#include <DmxSimple.h>                               // import DmxSimple library

int tictactoe[3][3] = {0};                           // initialize matrix for result calculation

int brightness = 64;

// define Colors for 7 channels DMX fixture
int aus[7]   = {brightness,   0,   0,   0, 0, 0, 0};        // define rgb color off (master full, RGB off)
int rot[7]   = {brightness, 255,   0,   0, 0, 0, 0};        // define rgb color red (master full, red on)
int blau[7]  = {brightness,   0,   0, 255, 0, 0, 0};        // define rgb color blue (master full, blue on)
int gruen[7] = {brightness,   0, 255,   0, 0, 0, 0};        // define rgb color green for animations
int gelb[7]  = {brightness, 255, 255,   0, 0, 0, 0};        // define rgb color yellow for tie game
int weiss[7] = {brightness, 255, 255, 255, 0, 0, 0};        // define rgb color white for animations

int arrayPositionX[9] = {0, 0, 0, 1, 1, 1, 2, 2, 2}; // array coordinate for given lamp number
int arrayPositionY[9] = {0, 1, 2, 0, 1, 2, 0, 1, 2}; // array coordinate for given lamp number

int r[9] = { 2,  3,  4,  5,  6,  7,  8,  9,  10};     // define pin numbers for input red
int b[9] = {A2, A3, A4, A5, A6, A7, A8, A9, A10};     // define pin numbers for input blue

int lamp[9] = {1, 8, 15, 22, 29, 36, 43, 50, 57};     // define start adresses of the lamps

int maxChannels = 64;                                 // defining the maximum number of channels used

// idle stuff
unsigned long uptime_last_change;
int max_idle_time = 1000 * 300 * 1;                     // in milliseconds after if no one is playing go to animation

int last_tictactoe[3][3] = {0};
bool gameWon = false;                                 // Track if game is won
int currentAnimationScene = 0;                        // Track current animation scene

void setup() {
  Serial.begin(9600);                                 
  Serial.println("Start");
  print_tictactoe();                                  
  delay(1000);

  // initialize Inputs
  for (int i = 0; i < 9; i++)                          
  {
    pinMode(r[i], INPUT_PULLUP); 
    pinMode(b[i], INPUT_PULLUP);
  }
  
  DmxSimple.usePin(11);                               
  DmxSimple.maxChannel(maxChannels);                  

// set all channes to black
  for (int i = 0; i < maxChannels; i++)               
  {
    DmxSimple.write(i, 0);
  }
  
  uptime_last_change = millis();  // Initialize idle timer
}

void loop() {
  readPins_writeTictactoe();

  int scene;
  print_tictactoe();      
  scene = check_win();    
  
  // Check for tie game
  if(scene == -1 && isBoardFull()) {
    tie_animation();
    gameWon = true;
  }
  
  // if no one is playing for a long time, go to animation
  if(no_player_present() == 1)
  {
    Serial.print("We have no player. Starting idle animation.\n");
    idle_animation();
  }

  // If game is won, wait for board to be cleared
  if(gameWon) {
    if(isBoardEmpty()) {
      gameWon = false;
      Serial.println("Board cleared. Ready for new game!");
    }
  }

  delay(100);
}

void readPins_writeTictactoe()
{
  for(int i = 0; i < 9; i++)          
  {
    if (!digitalRead(r[i]) && digitalRead(b[i]))  // Only red connected
    {
      if(tictactoe[arrayPositionX[i]][arrayPositionY[i]] == 0 && !gameWon) { // Only allow if spot is empty and game not won
        setLamp(i, rot);        
        tictactoe[arrayPositionX[i]][arrayPositionY[i]] = 1;    
      }
    }
    else if (digitalRead(r[i]) && !digitalRead(b[i]))  // Only blue connected
    {
      if(tictactoe[arrayPositionX[i]][arrayPositionY[i]] == 0 && !gameWon) { // Only allow if spot is empty and game not won
        setLamp(i, blau);       
        tictactoe[arrayPositionX[i]][arrayPositionY[i]] = -1;  
      }
    }
    else if (digitalRead(r[i]) && digitalRead(b[i])) // Nothing connected
    {
      setLamp(i, aus);          
      tictactoe[arrayPositionX[i]][arrayPositionY[i]] = 0;   
    }
    // If both are connected (error state), keep current state
  }
}

bool isBoardFull() {
  for(int row = 0; row < 3; row++) {
    for(int col = 0; col < 3; col++) {
      if(tictactoe[row][col] == 0) {
        return false;
      }
    }
  }
  return true;
}

bool isBoardEmpty() {
  for(int row = 0; row < 3; row++) {
    for(int col = 0; col < 3; col++) {
      if(tictactoe[row][col] != 0) {
        return false;
      }
    }
  }
  return true;
}

int no_player_present()
{
  if(areNotEqual(last_tictactoe, tictactoe) == 0) 
  {
    uptime_last_change = millis();
    for (int row = 0; row < 3; row++)
    {
      for (int column = 0; column < 3; column++)
      {
         last_tictactoe[row][column] = tictactoe[row][column];
      }
    }
    Serial.print("change detected.\n");
    return 2;
  }
  else
  {
    if(millis() - uptime_last_change > max_idle_time)
    {
      return 1;
    }
  }
  return 0;
}

int areNotEqual(int (*arr1)[3], int (*arr2)[3])
{
  for(int row = 0; row < 3; row++)
  {
    for(int column = 0; column < 3; column++)
    {
        if(last_tictactoe[row][column] != tictactoe[row][column])
        {
          return 0;
        }
    }
  }
  return 1;
}

void setLamp(int channel, int color[])      
{
  for(int i = 0; i < 7; i++) {
    DmxSimple.write(lamp[channel] + i, color[i]);
  }
}

void win_animation(int total, int typ, int position)
{
  int* color;  // Changed to pointer
  

  // Output to serial connection and set color
  if(total == -3){
    Serial.print("Blue wins! "); 
    color = blau;
  }
  if(total == 3){
    Serial.print("Red wins! "); 
    color = rot;
  }
  
  if(position == 0){Serial.print("In the first ");}
  if(position == 1){Serial.print("In the second ");}
  if(position == 2){Serial.print("In the third ");}
  if(typ == 0){Serial.print("row.");}
  if(typ == 1){Serial.print("column.");}
  if(typ == 2){Serial.print("diagonal.");}
  Serial.println();

  // blink the winner series
  Serial.print("Blink the winning positions.\n");
  int no_of_blinks = 10; 
  int on_time = 400;     
  int off_time = 400;    

  for (int n = 0; n < no_of_blinks; n++)
  {
    // light up the 3 winning lights in right color
    for (int i = 0; i < 3; i++ )
    {
      if(typ == 0) // row win
      {
        if (position==0){
          setLamp(0,color);
          setLamp(1,color);
          setLamp(2,color);
          Serial.println("an");
        }
        if (position == 1){
          setLamp(3,color);
          setLamp(4,color);
          setLamp(5,color);
          Serial.println("an");
        }
        if (position == 2){
          setLamp(6,color);
          setLamp(7,color);
          setLamp(8,color);
          Serial.println("an");
        }

      }
      if(typ == 1) // column win
      {
        if (position==0){
          setLamp(0,color);
          setLamp(3,color);
          setLamp(6,color);
          Serial.println("an");
        }
        if (position == 1){
          setLamp(1,color);
          setLamp(4,color);
          setLamp(7,color);
          Serial.println("an");
        }
        if (position == 2){
          setLamp(2,color);
          setLamp(6,color);
          setLamp(8,color);
          Serial.println("an");
        }        






      }
      if(typ == 2) // diagonal win
      {
        if(position == 0)
        {
          setLamp(0,color);
          setLamp(4,color);
          setLamp(7,color);
          Serial.println("an");
         }
        if(position == 1)
        {
          setLamp(2,color);
          setLamp(4,color);
          setLamp(6,color);
          Serial.println("an");
        }
      }
    }
    delay(on_time);

    // all lights off
    for (int n = 0; n < 9; n++)  
    {
      setLamp(n, aus);
      Serial.println("aus");
    }
    delay(off_time);
  }
  
  gameWon = true;  // Mark game as won
}

void tie_animation() {
  Serial.println("It's a tie!");
  
  // Flash all lights yellow
  for(int i = 0; i < 5; i++) {
    for(int n = 0; n < 9; n++) {
      setLamp(n, gelb);
      Serial.println("yellow");
    }
    delay(300);
    for(int n = 0; n < 9; n++) {
      setLamp(n, aus);
      Serial.println("off");
    }
    delay(300);
  }
}

int check_win()
{
  int total = 0;

  // check rows
  for (int row = 0; row < 3; row++)
  {
   for (int column = 0; column < 3; column++)
   {
     total += tictactoe[row][column];
   }
   if (abs(total) == 3)
   {
    win_animation(total, 0, row);
    return row;
   }
   else total = 0;
  }

  // check columns
  for (int column = 0; column < 3; column++)
  {
   for (int row = 0; row < 3; row++)
   {
     total += tictactoe[row][column];
   }
   if (abs(total) == 3)
   {
    win_animation(total, 1, column);
    return column + 3;
   }
   else total = 0;
  }

  // check main diagonal
  for (int i = 0; i < 3; i++)
  {
    total += tictactoe[i][i];
  }
  if (abs(total) == 3)
  {
    win_animation(total, 2, 0);
    return 6;
  }
  else total = 0;

  // check anti-diagonal
  for (int i = 0; i < 3; i++)
  {
    total += tictactoe[2 - i][i];
  }
  if (abs(total) == 3)
  {
    win_animation(total, 2, 1);
    return 7;
  }

  return -1;
}

void idle_animation() {
  Serial.println("Running idle animation...");
  
  // Keep checking for player activity
  while(no_player_present() == 1) {
    
    currentAnimationScene = 0; //(currentAnimationScene + 1) % 4;  // Cycle through 4 scenes
    
    switch(currentAnimationScene) {
      case 0:  // Wave pattern
        for(int i = 0; i < 9; i++) {
          setLamp(i, gruen);
          Serial.print(i + " ");
          Serial.println("idleOn");
          delay(1000);
          setLamp(i, aus);
          Serial.println("idle_off");
          if(no_player_present() != 1) break;
        }
        break;
        
      case 1:  // Checkerboard
        for(int i = 0; i < 9; i++) {
          if(i % 2 == 0) setLamp(i, rot);
          else setLamp(i, blau);
        }
        delay(500);
        if(no_player_present() != 1) return;
        
        for(int i = 0; i < 9; i++) {
          if(i % 2 == 1) setLamp(i, rot);
          else setLamp(i, blau);
        }
        delay(500);
        break;
        
      case 2:  // Rainbow cycle
        for(int c = 0; c < 3; c++) {
          for(int i = 0; i < 9; i++) {
            if(c == 0) setLamp(i, rot);
            else if(c == 1) setLamp(i, gruen);
            else setLamp(i, blau);
          }
          delay(400);
          if(no_player_present() != 1) return;
        }
        break;
        
      case 3:  // Spiral
        int spiral[9] = {0, 1, 2, 5, 8, 7, 6, 3, 4};
        for(int i = 0; i < 9; i++) {
          setLamp(spiral[i], weiss);
          delay(100);
          setLamp(spiral[i], aus);
          if(no_player_present() != 1) return;
        }
        break;
    }
  }
  
  // Clear all lights when returning to game
  for(int i = 0; i < 9; i++) {
    setLamp(i, aus);
  }
}

void print_tictactoe()
{
 Serial.print(tictactoe[arrayPositionX[0]][arrayPositionY[0]]);
 Serial.print(" ");
 Serial.print(tictactoe[arrayPositionX[1]][arrayPositionY[1]]);
 Serial.print(" ");
 Serial.println(tictactoe[arrayPositionX[2]][arrayPositionY[2]]);

 Serial.print(tictactoe[arrayPositionX[3]][arrayPositionY[3]]);
 Serial.print(" ");
 Serial.print(tictactoe[arrayPositionX[4]][arrayPositionY[4]]);
 Serial.print(" ");
 Serial.println(tictactoe[arrayPositionX[5]][arrayPositionY[5]]);

 Serial.print(tictactoe[arrayPositionX[6]][arrayPositionY[6]]);
 Serial.print(" ");
 Serial.print(tictactoe[arrayPositionX[7]][arrayPositionY[7]]);
 Serial.print(" ");
 Serial.println(tictactoe[arrayPositionX[8]][arrayPositionY[8]]);
 Serial.println("");
}