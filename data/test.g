; test program: home then initialize, perform programmed moves and gracefully finish program

; home, absolute positioning, stop spindle/servo, disable servo signal
$H
G90
M5
S0

; feed rate, unit is mm, work in machine coordinates
F500
G21
G53

; set current position manually to (X,Z)=(0,0), move to position (0,0): eliminates one GRBL error message
G92 X0 Z0
G1 X0 Z0

; enable spindle/servo, move servo to min pos, disable stepper driver idling
M3
S1
$1=255

; Z0
Z0

; move, dwell, activate servo, dwell
X10
G4 P1.0
S1000
G4 P0.25
S1

X20
G4 P1.0
S1000
G4 P0.25
S1

X30
G4 P1.0
S1000
G4 P0.25
S1

; Z10
Z10

X20
G4 P1.0
S1000
G4 P0.25
S1

X10
G4 P1.0
S1000
G4 P0.25
S1

X0
G4 P1.0
S1000
G4 P0.25
S1

; wave
S1
S1000
S1

; re-enable stepper driver idling to 25ms; request movement of 0 to activate idling
$1=25
G91
Z+0.01
Z-0.01

; stop servo signal, stop spindle, end program
S0
M5
M2

; program finished
