# Arduino_P9813
A small program to use P9813 LEDs with an Arduino Uno

Please leave a note/comment or file an issue, if you use this code and it works for you! It's "postcard-ware" :).

Copy the file into a directory, and rename the directory and the file with the same name to make the Arduino IDE to be happy.
The code is for an Uno but will work with anything Arduino or AVR that can compile the code. You can delete the heartbeat code
if there's no built in LED in your board.

I'm contributing this because it was hard to decode the P9813 data sheet with barely any explanations that were also in chinese.
I believe FastLED supports the P9813 chipset but this is bare bones and requires nothing else to run.

My experience about the P9813 LEDs I have in stock:
- They're sensitive to power noise
- The brightness flickers if it's too low, so I use it max brightness
- They seem to run 5V to 24V, but I run mine at 12V. At 5V they glitch and don't process the data well.

Enjoy!
