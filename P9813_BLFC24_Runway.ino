// Basic P9813 LED strip code - no library required!
//
// (c) 2024 Copyright Dan Truong Sept 2024
//
// This code lights up all the LEDs blue with a white tracer pixel running
// along them to simulate a Sci-fi landing strip pattern.
//
///////////////////////////////////////////////////////////////////////////////
// P9813 pixel protocol:
// =====================
// 4 bytes per pixel: { Header, Blue, Red, Green }
// Header one byte: {1,1, !b7, !b6, !r7, !r6, !g7, !g6}
// where b7 is the most significant bit of the blue byte, etc.
//
// The LED strip is reset by sending 4 bytes set to zero.
// If the data is incorrect for a given pixel, its color stays unchanged.
// Valid color pixels will however be updated properly.
//
// For example: A Black pixel is 0xFF,0x00,0x00,0x00
// For example: A White pixel is 0xC0,0xFF,0xFF,0xFF
//
// P9813 communication protocol:
// =============================
// Send one bit at a time through the DATA pin.
// The bit must be set AFTER the CLOCK pin is set to LOW (0V)
// The P9813 controller latches (records) a bit when the CLOCK changes to HIGH.
///////////////////////////////////////////////////////////////////////////////

#define DATA_PIN  6
#define CLK_PIN   7
#define NUM_PIXELS 64 // There's no limit on pixels but it'll go slower

void setup()
{
  //Serial.begin(9600); // Debug
  pinMode(LED_BUILTIN, OUTPUT); // Debug Heartbeat: blink the Uno's built-in LED
  pinMode(CLK_PIN, OUTPUT);     // P9813 CLOCK pin
  pinMode(DATA_PIN, OUTPUT);    // P9813 DATA pin

  // Not really needed: junk data
  ledStartFrame(HIGH);

  // Turn off the LED strip
  ledStartFrame(LOW);
    for(int16_t i = 0; i < NUM_PIXELS+1; i++) {
      pushAPixel(0, 0, 0);
    }
}

void loop()
{
  // Current position of the white tracer
  static uint16_t whitePixelPos = 0;

  ledStartFrame(LOW);
  for(int16_t i = 0; i < NUM_PIXELS; i++) {
    if (whitePixelPos == i) {
      // White tracer
      pushAPixel(0xFF, 0xFF, 0xFF);
    } else {
      // Blue background
      pushAPixel(0, 0, 0xFF);
    }
  }

  // Loop the position of the white streamer
  if (++whitePixelPos >= NUM_PIXELS) {
    whitePixelPos = 0;
  }

  blinkOnboardLed(); // Heartbeat
  //delay(10); // If you have few LEDs the streamer will be too fast to see
}

// Debug: blink the artuino board (code heartbeat)
static inline void blinkOnboardLed()
{
  static bool isOn = false;
  isOn = !isOn;
  digitalWrite(LED_BUILTIN, (isOn != 0) ? HIGH : LOW);  // blink
}

// Version 2 of the the code to send out a pixel to the P9813
static inline void pushAPixel2(uint8_t r, uint8_t g, uint8_t b)
{
 // The P9813 header byte is a checksum we must compute
  const uint8_t cksum = ~((b & 0xC0) >> 2 | (g & 0xC0) >> 4 | (r & 0xC0) >> 6);
  //const uint8_t cksum = 0xC0 | (~b & 0xC0) >> 2 | (~g & 0xC0) >> 4 | (~r & 0xC0) >> 6;

  // Generate 32bit pixel data
  union u_t {
    uint32_t word;    // Pixel manipulated as a word
    uint8_t bytes[4]; // Pixel manipulated as 4 bytes
  };
  const u_t pixel = {.bytes = {r, g, b, cksum}};

  // Optimization: Only update the DATA pin if its value changed (IO is slow!).
  uint8_t lastBit = 0x3;

  for(int8_t i = 31; i >= 0; i--) {
    // Extract a pixel bit to bitbang to the port
    const uint8_t bit = (pixel.word >> i) & 0x01;
    //Serial.print(bit); // Debug
    digitalWrite(CLK_PIN, LOW);
    if (bit != lastBit) {
      digitalWrite(DATA_PIN, (bit != 0) ? HIGH : LOW);
    }
    digitalWrite(CLK_PIN, HIGH);
  }
  //Serial.print(" "); // Debug
  //Serial.println(pixel.word); // Debug
}

// Send a pixel to the P9813
static inline void pushAPixel(uint8_t r, uint8_t g, uint8_t b)
{
  // The P9813 header byte is a checksum we must compute
  const uint8_t cksum = ~((b & 0xC0) >> 2 | (g & 0xC0) >> 4 | (r & 0xC0) >> 6);
  //const uint8_t cksum = 0xC0 | (~b & 0xC0) >> 2 | (~g & 0xC0) >> 4 | (~r & 0xC0) >> 6;

  // Optimization: Only update the DATA pin if its value changed (IO is slow!).
  uint8_t lastBit = 0x3;

  for(int8_t i = 7; i >= 0; i--) {
    const uint8_t bit = (cksum >> i) & 0x01;
    digitalWrite(CLK_PIN, LOW);
    if (bit != lastBit) {
      digitalWrite(DATA_PIN, bit);
      lastBit = bit;
    }
    digitalWrite(CLK_PIN, HIGH);
  }
  for(int8_t i = 7; i >= 0; i--) {
    const uint8_t bit = (b >> i) & 0x01;
    digitalWrite(CLK_PIN, LOW);
    if (bit != lastBit) {
      digitalWrite(DATA_PIN, bit);
      lastBit = bit;
    }
    digitalWrite(CLK_PIN, HIGH);
  }
  for(int8_t i = 7; i >= 0; i--) {
    const uint8_t bit = (g >> i) & 0x01;
    digitalWrite(CLK_PIN, LOW);
    if (bit != lastBit) {
      digitalWrite(DATA_PIN, bit);
      lastBit = bit;
    }
    digitalWrite(CLK_PIN, HIGH);
  }
  for(int8_t i = 7; i >= 0; i--) {
    const uint8_t bit = (r >> i) & 0x01;
    digitalWrite(CLK_PIN, LOW);
    if (bit != lastBit) {
      digitalWrite(DATA_PIN, bit);
      lastBit = bit;
    }
    digitalWrite(CLK_PIN, HIGH);
  }
  digitalWrite(CLK_PIN, LOW);
}

// Reset the LED strip by sending 4 bytes set to zero.
// This allows refreshing all the LEDs. Otherwise we'd keep sending pixel data
// down the LED strip indefinitely.
static inline void ledStartFrame(const uint8_t dataLevel)
{
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(DATA_PIN, dataLevel);
  // We write the same bit value 32 times, so we don't need to update the DATA
  // pin. We just need to make the clock flip 32 times to latch the value into
  // the P9813 32 times.
  for(int8_t i = 31; i >= 0; i--) {
    digitalWrite(CLK_PIN, HIGH);
  digitalWrite(CLK_PIN, LOW);
  }
}