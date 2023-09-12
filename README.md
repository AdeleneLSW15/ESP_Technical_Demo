# ESP Technical Demo

In Embedded Systems Project, technical demonstration criteria is assessed. Based on the task achieved, the score will be logged and taken into account to the unit grade result.

- TD1: Assess whether your buggy is able to perform basic Motor Control.
- TD2: Assess sensors for your project. Sensors does not have to be fitted to the buggy.
- TD3: Assess autonomous white-line following control capability of your buggy - Steering. 
- TD4: Assess autonomous white-line following buggy on mini race track. Make the buggy robust.

For TD4, I have used the TD3 code files but with upgraded speed. Furthermore, also supplied additional TCRT5000 sensors for backup.

Additionally, I have also created debugging code files that test out initial features.
- Debugging sensors file allows us to take the voltage reading of TCRT5000 sensor when placed at a white or black surface. This allows me to record the voltage threshold to allow white-line following control using sensor values.
- TD3_P_Value: Code file that allows us to record the best performing proprotional constant for speed control. Left and right potentiometer on STM32 is used for tuning the optimal value for the motors. Display LCD screen tells us the kp value for both motors. This method reduces the amount of compilation when tuning proportional constant, saving time in development.
