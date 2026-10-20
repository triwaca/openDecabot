# Teste do Display OLED SSD1306 no openDecabot com ESP32-C3
# Pinos I2C: SDA = GPIO 8, SCL = GPIO 9

from machine import Pin, I2C
import time
import ssd1306

# Inicializa I2C nos pinos do openDecabot ESP32-C3
i2c = I2C(0, sda=Pin(8), scl=Pin(9), freq=400000)

WIDTH = 128
HEIGHT = 64

oled = ssd1306.SSD1306_I2C(WIDTH, HEIGHT, i2c, addr=0x3C)

def desenha_olhos(shift_x=0, blink_step=0):
    oled.fill(0)
    
    eye_w = 26
    max_eye_h = 36
    # blink_step: 0 (aberto) a 4 (fechado)
    eye_h = max(4, max_eye_h - (blink_step * 8))
    eye_y = (HEIGHT - eye_h) // 2

    left_x = 36 + (shift_x * 10) - (eye_w // 2)
    right_x = 92 + (shift_x * 10) - (eye_w // 2)

    # Desenha os dois olhos
    oled.fill_rect(left_x, eye_y, eye_w, eye_h, 1)
    oled.fill_rect(right_x, eye_y, eye_w, eye_h, 1)
    
    oled.show()

# Mensagem inicial
oled.fill(0)
oled.text("openDecabot", 20, 15, 1)
oled.text("ESP32-C3 RISC-V", 4, 32, 1)
oled.text("MicroPython", 20, 48, 1)
oled.show()
time.sleep(2)

# Loop de animação dos olhos
while True:
    # Olhando para frente
    desenha_olhos(0, 0)
    time.sleep(1.5)

    # Piscar de olhos
    for b in [1, 2, 3, 4, 3, 2, 1, 0]:
        desenha_olhos(0, b)
        time.sleep(0.04)

    time.sleep(1)

    # Olhar para a esquerda
    desenha_olhos(-1, 0)
    time.sleep(1.2)

    # Olhar para a direita
    desenha_olhos(1, 0)
    time.sleep(1.2)

