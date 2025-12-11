# TicTacToe-Arcade
A 2-player LED TicTacToe game built with NeoPixels, an LCD1602, and physical buttons.
Fully implemented on Arduino. (simulated using TinkerCas)

Features
- Two-player real-time gameplay (Red vs Blue)
- 9-pixel NeoPixel grid for visual game board
- LCD1602 display for turn notifications and win/tie messages
- Detects wins
- Detects ties
- Alternates turns
- Tracks total score
- Reset with physical button press
- Debounced, accurate input reading
- Modular code structure (display, buttons, LEDs, game logic separated)

Hardware Required:
- Arduino Uno / Nano
- 9-pixel NeoPixel strip or matrix
- LCD1602 
- 9 push buttons (for grid)
- Jumper wires
- Breadboard
