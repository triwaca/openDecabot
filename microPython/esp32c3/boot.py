# This file is executed on every boot (including wake-boot from deepsleep)
# import esp
# esp.osdebug(None)
#import webrepl
#webrepl.start()
import machine
import time
#from machine import Pin, I2C
#import ssd1306
#display.text('Hello, World!', 0, 0, 1)
#display.show()
# using default address 0x3C
#i2c = I2C(sda=Pin(8), scl=Pin(9))
#display = ssd1306.SSD1306_I2C(128, 64, i2c)
#from machine import Pin
# Buzzer desativado por seguranca no boot (evita sobrecorrente no GPIO 3)
# buzzer_pin = machine.Pin(3, machine.Pin.OUT)
# buzzer = machine.PWM(buzzer_pin)
# buzzer.duty_u16(32768)
# buzzer.freq(440)
# time.sleep(1)
# buzzer.duty(0)
# buzzer.deinit()
print(" openDecabot ESP32 C3 pinout")
print("       5 ●┌─┤██████├─┐● 5V")
print(" Servo 6 ●│ │██████│ │● GND")
print(" Servo 7 ●│ └▀▀▀▀▀▀┘ │● 3.3V")
print("OLED SDA ●│[BOT][RST]│● 4")
print("OLED SCL ●│          │● 3 - Buzzer")
print("      10 ●│          │● 2")
print("      TX ●│          │● 1")
print("      RX ●│          │● 0")
print("          └──────────┘ ")
