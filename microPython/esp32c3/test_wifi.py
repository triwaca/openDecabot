# Teste isolado de Wi-Fi no ESP32-C3 SuperMini
import network
import time

print("1. Desligando interfaces antigas...")
sta = network.WLAN(network.STA_IF)
sta.active(False)

ap = network.WLAN(network.AP_IF)
ap.active(False)
time.sleep_ms(300)

print("2. Ativando Ponto de Acesso (AP)...")
ap.active(True)
time.sleep_ms(500)

# Ajuste de estabilidade para ESP32-C3 SuperMini
try:
    ap.config(txpower=8.5) # Reduz potência excessiva para evitar queda de tensão no chip
except Exception as e:
    print("txpower:", e)

try:
    ap.config(channel=6)
except Exception as e:
    print("channel:", e)

# Configura com senha de 8 digitos (WPA2) - garante que o rádio transmita e o celular encontre
SSID_NAME = "MeuDecabot"
WIFI_PASS = "Bruno007"

try:
    ap.config(essid=SSID_NAME, password=WIFI_PASS, authmode=3) # 3 = WPA2-PSK
    print(f"Rede configurada: {SSID_NAME} com senha: {WIFI_PASS}")
except Exception as e:
    print("Erro ao configurar com senha, tentando modo aberto:", e)
    ap.config(essid=SSID_NAME)

time.sleep(1)

print("\n--- STATUS DO WI-FI ---")
print("AP Ativo?   :", ap.active())
print("IP da Placa :", ap.ifconfig()[0])
print("Nome (SSID) :", ap.config('essid'))
print("Canal       :", ap.config('channel'))
print("-----------------------")
print("Pegue seu celular agora e procure por 'MeuDecabot'!")

