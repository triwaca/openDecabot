# ==============================================================================
# openDecabot - Firmware Leve & Otimizado para ESP32-C3 SuperMini
# ==============================================================================
# Recursos:
#  - Wi-Fi Access Point ultrarrápido (BmoDecabot - 192.168.4.1)
#  - Servidor DNS Cativo (Captive Portal automático)
#  - Servidor Web HTTP ultra-leve com controles em tempo real de latência ZERO
#  - Interface mobile moderna com D-pad, Olhos BMO, Buzina e Passeio Autônomo
#  - Animação expressiva dos olhos no display OLED SSD1306 (não-bloqueante)
#  - Controle ágil dos 2 servomotores 360° sem atraso e sem queda de tensão
#  - Modo Passeio Autônomo inteligente e não-bloqueante
# ==============================================================================

import gc
import machine
import network
import socket
import time
from machine import Pin, PWM, I2C

gc.collect()

# ------------------------------------------------------------------------------
# 1. PINAGEM DO ESP32-C3 SUPERMINI (openDecabot)
# ------------------------------------------------------------------------------
PIN_SERVO_DIR = 6   # GPIO 6 (Servo Direito)
PIN_SERVO_ESQ = 7   # GPIO 7 (Servo Esquerdo)
PIN_OLED_SDA  = 8   # GPIO 8 (I2C SDA)
PIN_OLED_SCL  = 9   # GPIO 9 (I2C SCL)
PIN_BUZZER    = -1  # Desativado por padrão para evitar ruído / sobrecorrente

SSID_NAME     = "BmoDecabot"
WIFI_PASS     = ""  # Rede aberta para conexão instantânea sem senha

# ------------------------------------------------------------------------------
# 2. INICIALIZAÇÃO DO WI-FI ACCESS POINT
# ------------------------------------------------------------------------------
try:
    sta = network.WLAN(network.STA_IF)
    sta.active(False)
except Exception:
    pass

ap = network.WLAN(network.AP_IF)
ap.active(False)
time.sleep_ms(150)
ap.active(True)
time.sleep_ms(250)

try:
    ap.config(txpower=8.5) # Limita pico de corrente do rádio (evita brownout)
    ap.config(channel=6)
    if WIFI_PASS:
        ap.config(essid=SSID_NAME, password=WIFI_PASS, authmode=network.AUTH_WPA_WPA2_PSK)
    else:
        ap.config(essid=SSID_NAME, authmode=network.AUTH_OPEN)
except Exception:
    pass

print("\n------------------------------------------")
print("openDecabot Wi-Fi ativo!")
print("Nome da Rede :", SSID_NAME)
print("IP da Placa  : 192.168.4.1")
print("IP no browser: http://192.168.4.1")
print("------------------------------------------")

# ------------------------------------------------------------------------------
# 3. DRIVER DO BUZZER (Seguro & Opcional)
# ------------------------------------------------------------------------------
buzzer_pwm = None

def beep(freq=880, duration_ms=80):
    global buzzer_pwm
    if PIN_BUZZER < 0 or duration_ms <= 0:
        return
    try:
        if buzzer_pwm is None:
            buzzer_pwm = PWM(Pin(PIN_BUZZER), freq=1000, duty_u16=0)
        buzzer_pwm.freq(max(20, min(int(freq), 12000)))
        buzzer_pwm.duty_u16(32768)
        time.sleep_ms(duration_ms)
        buzzer_pwm.duty_u16(0)
    except Exception:
        pass

# ------------------------------------------------------------------------------
# 4. DISPLAY OLED SSD1306 & ANIMAÇÃO EXPRESSIVA NÃO-BLOQUEANTE
# ------------------------------------------------------------------------------
oled = None
try:
    try:
        from ssd1306 import SSD1306_I2C
        i2c = I2C(0, sda=Pin(PIN_OLED_SDA), scl=Pin(PIN_OLED_SCL), freq=400000)
        oled = SSD1306_I2C(128, 64, i2c, addr=0x3C)
        print("Display OLED detectado com sucesso via Hardware I2C!")
    except Exception:
        from machine import SoftI2C
        from ssd1306 import SSD1306_I2C
        i2c = SoftI2C(sda=Pin(PIN_OLED_SDA), scl=Pin(PIN_OLED_SCL), freq=400000)
        oled = SSD1306_I2C(128, 64, i2c, addr=0x3C)
        print("Display OLED detectado com sucesso via SoftI2C!")
except Exception as e:
    print("OLED não detectado:", e)

# Variáveis de estado dos olhos
eye_cur_x = 0
eye_cur_y = 0
eye_cur_mood = "NORMAL"

def desenha_olhos(shift_x=0, shift_y=0, blink_l=0, blink_r=0, mood="NORMAL", scale_l=0, scale_r=0):
    """Renderiza olhos expressivos estilo BMO / Vector com cantos suaves, brilho e reações"""
    if not oled:
        return
    global eye_cur_x, eye_cur_y, eye_cur_mood
    eye_cur_x, eye_cur_y, eye_cur_mood = shift_x, shift_y, mood

    oled.fill(0)
    cx_l = 38 + shift_x
    cx_r = 90 + shift_x
    cy = 32 + shift_y

    for is_left, cx, blink, sc in ((True, cx_l, blink_l, scale_l), (False, cx_r, blink_r, scale_r)):
        w = max(8, 30 + sc)
        base_h = 36 + sc
        if blink > 0:
            h = max(2, base_h - (blink * 7))
        else:
            h = base_h

        x = cx - (w // 2)
        y = cy - (h // 2)

        if h <= 4:
            oled.fill_rect(x, cy - 2, w, 4, 1)
            continue

        # Retângulo com cantos arredondados suaves
        r = 5 if h >= 18 else 2
        oled.fill_rect(x + r, y, w - 2 * r, h, 1)
        oled.fill_rect(x, y + r, w, h - 2 * r, 1)
        oled.fill_rect(x + 2, y + 1, w - 4, h - 2, 1)
        oled.fill_rect(x + 1, y + 2, w - 2, h - 4, 1)

        # Expressões especiais
        if mood == "FELIZ" and h >= 16:
            # Olhos sorridentes arqueados (^ ^)
            cut = h // 2
            oled.fill_rect(x + 2, y + h - cut + 2, w - 4, cut, 0)
            oled.fill_rect(x + 5, y + h - cut, w - 10, cut, 0)
        elif mood == "FOCADO" and h >= 16:
            # Olhar determinado ao acelerar (> <)
            for i in range(10):
                if is_left:
                    oled.hline(x + (i * 2), y + (i // 2), w - (i * 2), 0)
                else:
                    oled.hline(x, y + (i // 2), w - (i * 2), 0)

    oled.show()

# Fila de animações de olhos assíncrona (100% livre de sleep bloqueante!)
anim_queue = []
anim_proximo_tempo = 0

def enfileirar_animacao(frames):
    global anim_queue, anim_proximo_tempo
    anim_queue = frames[:]
    anim_proximo_tempo = time.ticks_add(time.ticks_ms(), frames[0][0]) if frames else 0

def processar_animacao(agora):
    global anim_queue, anim_proximo_tempo
    if not anim_queue:
        return
    if time.ticks_diff(agora, anim_proximo_tempo) >= 0:
        delay, func, args = anim_queue.pop(0)
        try:
            func(*args)
        except Exception:
            pass
        if anim_queue:
            anim_proximo_tempo = time.ticks_add(agora, anim_queue[0][0])

def piscar_olhos(duplo=False):
    global eye_cur_x, eye_cur_y, eye_cur_mood
    x, y, m = eye_cur_x, eye_cur_y, eye_cur_mood
    frames = [
        (0,  desenha_olhos, (x, y, 3, 3, m, 0, 0)),
        (35, desenha_olhos, (x, y, 5, 5, m, 0, 0)),
        (45, desenha_olhos, (x, y, 0, 0, m, 0, 0)),
    ]
    if duplo:
        frames.extend([
            (70, desenha_olhos, (x, y, 4, 4, m, 0, 0)),
            (45, desenha_olhos, (x, y, 0, 0, m, 0, 0)),
        ])
    enfileirar_animacao(frames)

def piscar_um_olho():
    global eye_cur_x, eye_cur_y
    x, y = eye_cur_x, eye_cur_y
    frames = [
        (0,  desenha_olhos, (x, y, 0, 3, "FELIZ", 0, 0)),
        (40, desenha_olhos, (x, y, 0, 5, "FELIZ", 0, 0)),
        (60, desenha_olhos, (x, y, 0, 0, "NORMAL", 0, 0)),
    ]
    enfileirar_animacao(frames)

def olhar_suave(alvo_x, alvo_y, mood="NORMAL", sc_l=0, sc_r=0):
    global eye_cur_x, eye_cur_y
    mid_x = (eye_cur_x + alvo_x) // 2
    mid_y = (eye_cur_y + alvo_y) // 2
    frames = [
        (0,  desenha_olhos, (mid_x, mid_y, 0, 0, mood, sc_l // 2, sc_r // 2)),
        (35, desenha_olhos, (alvo_x, alvo_y, 0, 0, mood, sc_l, sc_r)),
    ]
    enfileirar_animacao(frames)

def animar_boot():
    if not oled:
        return
    for h in (4, 12, 24, 36):
        oled.fill(0)
        for cx in (38, 90):
            oled.fill_rect(cx - 15, 32 - (h // 2), 30, h, 1)
        oled.show()
        time.sleep_ms(25)
    desenha_olhos(0, -3, 0, 0, "FELIZ")
    time.sleep_ms(250)
    desenha_olhos(0, 0, 0, 0, "NORMAL")

def tela_espera():
    if not oled:
        return
    oled.fill(0)
    oled.rect(0, 0, 128, 64, 1)
    oled.text("openDecabot", 20, 6, 1)
    oled.hline(10, 18, 108, 1)
    oled.text("Conecte no WiFi:", 4, 25, 1)
    oled.text(SSID_NAME, 14, 38, 1)
    oled.text("192.168.4.1", 20, 50, 1)
    oled.show()

# ------------------------------------------------------------------------------
# 5. CONTROLE DOS SERVOS 360 COM RESPOSTA IMEDIATA & PROTEÇÃO ANTI-BROWNOUT
# ------------------------------------------------------------------------------
s_dir = PWM(Pin(PIN_SERVO_DIR), freq=50)
s_esq = PWM(Pin(PIN_SERVO_ESQ), freq=50)

STOP_DIR   = 4915
STOP_ESQ   = 4915
SPEED      = 1800    # Velocidade segura anti-brownout (faixa entre 1.0ms e 2.0ms)
SPEED_SLOW = SPEED // 2

cur_dir = STOP_DIR
cur_esq = STOP_ESQ
last_move_time = None
WATCHDOG_MS = 3500   # Watchdog de segurança caso o celular desconecte o Wi-Fi

def set_motores(dir_val, esq_val):
    """Acionamento direto e instantâneo dos servos (latência < 5ms) com proteção de partida"""
    global cur_dir, cur_esq
    if dir_val == cur_dir and esq_val == cur_esq:
        return
    # Amortecimento rápido de apenas 5ms ao sair do repouso para eliminar pico de partida
    if cur_dir == STOP_DIR and cur_esq == STOP_ESQ and (dir_val != STOP_DIR or esq_val != STOP_ESQ):
        s_dir.duty_u16((STOP_DIR + dir_val) // 2)
        s_esq.duty_u16((STOP_ESQ + esq_val) // 2)
        time.sleep_ms(5)
    cur_dir, cur_esq = dir_val, esq_val
    s_dir.duty_u16(cur_dir)
    s_esq.duty_u16(cur_esq)

def parar():
    """Parada 100% imediata dos motores sem nenhum atraso"""
    global last_move_time
    last_move_time = None
    set_motores(STOP_DIR, STOP_ESQ)
    desenha_olhos(0, 0, 0, 0, "NORMAL")

def frente():
    global last_move_time
    last_move_time = time.ticks_ms()
    set_motores(STOP_DIR - SPEED, STOP_ESQ + SPEED)

def tras():
    global last_move_time
    last_move_time = time.ticks_ms()
    set_motores(STOP_DIR + SPEED, STOP_ESQ - SPEED)

def esquerda():
    global last_move_time
    last_move_time = time.ticks_ms()
    set_motores(STOP_DIR - SPEED, STOP_ESQ - SPEED)

def direita():
    global last_move_time
    last_move_time = time.ticks_ms()
    set_motores(STOP_DIR + SPEED, STOP_ESQ + SPEED)

def frente_esq():
    global last_move_time
    last_move_time = time.ticks_ms()
    set_motores(STOP_DIR - SPEED, STOP_ESQ + SPEED_SLOW)

def frente_dir():
    global last_move_time
    last_move_time = time.ticks_ms()
    set_motores(STOP_DIR - SPEED_SLOW, STOP_ESQ + SPEED)

def tras_esq():
    global last_move_time
    last_move_time = time.ticks_ms()
    set_motores(STOP_DIR + SPEED, STOP_ESQ - SPEED_SLOW)

def tras_dir():
    global last_move_time
    last_move_time = time.ticks_ms()
    set_motores(STOP_DIR + SPEED_SLOW, STOP_ESQ - SPEED)

# Garante motores 100% parados ao iniciar
s_dir.duty_u16(STOP_DIR)
s_esq.duty_u16(STOP_ESQ)
animar_boot()

# ------------------------------------------------------------------------------
# 6. MODO PASSEIO AUTÔNOMO LEVE & NÃO-BLOQUEANTE
# ------------------------------------------------------------------------------
modo_autonomo = False
auto_estado = "PARAR"
auto_proximo_tempo = time.ticks_ms()
auto_passo = 0

def processar_autonomo(agora):
    global auto_estado, auto_proximo_tempo, auto_passo
    if not modo_autonomo:
        return
    if time.ticks_diff(agora, auto_proximo_tempo) < 0:
        return

    auto_passo = (auto_passo + 1) % 6
    if auto_passo in (0, 2, 4):
        frente()
        desenha_olhos(0, -3, 0, 0, "FOCADO", 0, 0)
        auto_estado = "FRENTE"
        auto_proximo_tempo = time.ticks_add(agora, 1600 + (auto_passo * 200))
    elif auto_passo == 1:
        esquerda()
        desenha_olhos(16, 0, 0, 0, "NORMAL", 4, -4)
        auto_estado = "ESQ"
        auto_proximo_tempo = time.ticks_add(agora, 650)
    elif auto_passo == 3:
        direita()
        desenha_olhos(-16, 0, 0, 0, "NORMAL", -4, 4)
        auto_estado = "DIR"
        auto_proximo_tempo = time.ticks_add(agora, 650)
    else:
        parar()
        olhar_suave(14, -2, "NORMAL", 3, -3)
        auto_estado = "PAUSA"
        auto_proximo_tempo = time.ticks_add(agora, 1100)

# ------------------------------------------------------------------------------
# 7. INTERFACE WEB HTML (Compacta, Responsiva & Sem Spam de Conexões)
# ------------------------------------------------------------------------------
HTML_PAGE = """<!DOCTYPE html><html lang="pt-BR"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no"><title>openDecabot</title>
<style>
:root{--bg:#070b14;--c:#00e5ff;--a:#ff7b00;--txt:#f1f5f9}
*{box-sizing:border-box;margin:0;padding:0;user-select:none;-webkit-user-select:none;touch-action:none}
body{background:radial-gradient(circle at 50% 20%,#1e293b,var(--bg));color:var(--txt);font-family:system-ui,-apple-system,sans-serif;display:flex;flex-direction:column;align-items:center;min-height:100vh;padding:12px 8px}
.hdr{display:flex;flex-direction:column;align-items:center;margin-bottom:6px}
.badge{font-size:10px;padding:2px 8px;border-radius:20px;background:rgba(52,211,153,.15);color:#34d399;font-weight:700;margin-bottom:2px}
h1{font-size:22px;letter-spacing:1px;background:linear-gradient(135deg,var(--c),var(--a));-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.visor{width:150px;height:54px;background:#000;border:2px solid #334155;border-radius:14px;display:flex;align-items:center;justify-content:center;gap:22px;margin:8px 0;box-shadow:0 0 10px rgba(0,229,255,.2)}
.eye{width:28px;height:28px;background:var(--c);border-radius:7px;box-shadow:0 0 8px var(--c);transition:transform .08s,height .08s}
.visor.blink .eye{height:3px}
.dpad{display:grid;grid-template-columns:repeat(3,68px);grid-template-rows:repeat(3,68px);gap:8px;margin:10px 0}
.btn{background:#1e293b;border:2px solid #334155;color:#fff;border-radius:14px;font-size:22px;display:flex;align-items:center;justify-content:center;cursor:pointer;box-shadow:0 4px #0f172a}
.btn:active,.btn.pressed{background:var(--a);transform:translateY(3px);box-shadow:none}
.btn.stop{background:#7f1d1d;border-color:#ef4444;color:#fee2e2}
.btn.diag{font-size:16px;color:#94a3b8}
.acts{display:grid;grid-template-columns:repeat(4,72px);gap:8px;margin-top:6px}
.act{background:#1e293b;border:1px solid #334155;color:#cbd5e1;padding:8px 2px;border-radius:10px;font-size:11px;font-weight:600;display:flex;flex-direction:column;align-items:center;gap:3px;cursor:pointer}
.act:active{background:var(--c);color:#000}
.act.on{background:#059669;border-color:#34d399;color:#fff}
.act span{font-size:18px}
.status{margin-top:12px;font-size:11px;font-weight:700;color:var(--c);letter-spacing:1px}
</style></head><body>
<div class="hdr"><div class="badge">&#9679; CONECTADO</div><h1>openDecabot</h1>
<div class="visor" id="v"><div class="eye" id="eL"></div><div class="eye" id="eR"></div></div></div>
<div class="dpad">
<button class="btn diag" data-c="frente_esq" data-x="-7" data-y="-7">&#x2196;</button>
<button class="btn" data-c="frente" data-x="0" data-y="-9">&#x25B2;</button>
<button class="btn diag" data-c="frente_dir" data-x="7" data-y="-7">&#x2197;</button>
<button class="btn" data-c="esquerda" data-x="-9" data-y="0">&#x25C0;</button>
<button class="btn stop" data-c="parar" data-x="0" data-y="0">&#x25A0;</button>
<button class="btn" data-c="direita" data-x="9" data-y="0">&#x25B6;</button>
<button class="btn diag" data-c="tras_esq" data-x="-7" data-y="7">&#x2199;</button>
<button class="btn" data-c="tras" data-x="0" data-y="9">&#x25BC;</button>
<button class="btn diag" data-c="tras_dir" data-x="7" data-y="7">&#x2198;</button>
</div>
<div class="acts">
<button class="act" id="btn-auto" onclick="toggleAuto()"><span>&#128694;</span>Passeio</button>
<button class="act" onclick="act('bip','Bip!')"><span>&#128266;</span>Buzina</button>
<button class="act" onclick="act('feliz','Feliz ^-^')"><span>&#128522;</span>Feliz</button>
<button class="act" onclick="act('piscar','Piscadinha!')"><span>&#128521;</span>Piscar</button>
</div>
<div class="status" id="st">PRONTO PARA COMANDO</div>
<script>
let cur=null,hbTimer=null,eL=document.getElementById("eL"),eR=document.getElementById("eR"),st=document.getElementById("st"),v=document.getElementById("v"),isAuto=false;
function setE(x,y){let t="translate("+x+"px,"+y+"px)";eL.style.transform=t;eR.style.transform=t}
function sendCmd(cmd){fetch("/"+cmd,{cache:"no-store"}).catch(()=>{})}
function start(c,x,y){
    if(cur===c)return;
    cur=c;
    setE(x,y);
    st.innerText="MOVENDO: "+c.toUpperCase().replace("_"," ");
    sendCmd(c);
    clearInterval(hbTimer);
    hbTimer=setInterval(()=>{if(cur)sendCmd(cur)},1500);
}
function stop(){
    clearInterval(hbTimer);
    if(cur!==null){
        cur=null;
        setE(0,0);
        st.innerText="PARADO";
        sendCmd("parar");
    }
}
function act(a,l){st.innerText=l;v.classList.add("blink");setTimeout(()=>v.classList.remove("blink"),250);sendCmd(a)}
function toggleAuto(){
    clearInterval(hbTimer);
    fetch("/autonomo",{cache:"no-store"}).then(r=>r.json()).then(d=>{
        let b=document.getElementById("btn-auto");
        isAuto=d.autonomo;
        if(isAuto){b.classList.add("on");st.innerText="MODO PASSEIO ATIVO"}
        else{b.classList.remove("on");st.innerText="PASSEIO DESATIVADO"}
    }).catch(()=>{})
}
document.querySelectorAll(".btn").forEach(b=>{
    let c=b.dataset.c,x=parseInt(b.dataset.x||0),y=parseInt(b.dataset.y||0);
    if(c==="parar"){
        b.addEventListener("pointerdown",e=>{e.preventDefault();stop()});
        return
    }
    b.addEventListener("pointerdown",e=>{
        e.preventDefault();
        b.classList.add("pressed");
        if(isAuto){document.getElementById("btn-auto").classList.remove("on");isAuto=false}
        start(c,x,y)
    });
    b.addEventListener("pointerup",e=>{e.preventDefault();b.classList.remove("pressed");stop()});
    b.addEventListener("pointercancel",e=>{b.classList.remove("pressed");stop()});
    b.addEventListener("pointerleave",e=>{if(b.classList.contains("pressed")){b.classList.remove("pressed");stop()}});
});
window.addEventListener("pointerup",()=>{document.querySelectorAll(".btn").forEach(b=>b.classList.remove("pressed"));stop()});
window.addEventListener("pointercancel",()=>{document.querySelectorAll(".btn").forEach(b=>b.classList.remove("pressed"));stop()});
window.addEventListener("touchend",()=>{document.querySelectorAll(".btn").forEach(b=>b.classList.remove("pressed"));stop()});
window.addEventListener("touchcancel",()=>{document.querySelectorAll(".btn").forEach(b=>b.classList.remove("pressed"));stop()});
window.addEventListener("blur",stop);
document.addEventListener("visibilitychange",()=>{if(document.hidden)stop()});
setInterval(()=>{if(!cur){v.classList.add("blink");setTimeout(()=>v.classList.remove("blink"),160)}},3500);
</script></body></html>"""

HTML_BYTES = HTML_PAGE.encode('utf-8')
HTML_LEN = len(HTML_BYTES)
del HTML_PAGE
gc.collect()

# ------------------------------------------------------------------------------
# 8. SERVIDOR DNS CATIVO (Captive Portal)
# ------------------------------------------------------------------------------
class CaptiveDNS:
    def __init__(self, ip='192.168.4.1'):
        self.ip_bytes = bytes([int(x) for x in ip.split('.')])
        self.sock = None
        for _ in range(3):
            try:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                self.sock.setblocking(False)
                self.sock.bind(('', 53))
                print("Servidor DNS Cativo ativo na porta 53!")
                break
            except Exception:
                if self.sock:
                    try: self.sock.close()
                    except Exception: pass
                    self.sock = None
                time.sleep_ms(200)

    def handle(self):
        if not self.sock:
            return
        try:
            while True:
                data, addr = self.sock.recvfrom(256)
                if len(data) < 12:
                    continue
                packet = bytearray()
                packet.extend(data[:2])           # Transaction ID
                packet.extend(b'\x81\x80')        # Standard query response
                packet.extend(data[4:6])          # Questions
                packet.extend(data[4:6])          # Answers
                packet.extend(b'\x00\x00\x00\x00')# RRs
                idx = 12
                while idx < len(data) and data[idx] != 0:
                    idx += 1 + data[idx]
                idx += 5
                packet.extend(data[12:idx])
                packet.extend(b'\xc0\x0c')        # Pointer
                packet.extend(b'\x00\x01\x00\x01')# A, IN
                packet.extend(b'\x00\x00\x00\x3c')# TTL: 60s
                packet.extend(b'\x00\x04')        # Length: 4 bytes
                packet.extend(self.ip_bytes)
                self.sock.sendto(packet, addr)
        except Exception:
            pass

dns = CaptiveDNS('192.168.4.1')

# ------------------------------------------------------------------------------
# 9. SERVIDOR WEB HTTP COM REUSO DE PORTA E LATÊNCIA MÍNIMA
# ------------------------------------------------------------------------------
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
except Exception:
    pass

porta_ok = False
for tentativa in range(5):
    try:
        server.bind(('', 80))
        porta_ok = True
        break
    except Exception as e:
        time.sleep_ms(300)

if not porta_ok:
    print("Aviso: Porta 80 ocupada. Reinicie o ESP32 pelo botão RST.")
else:
    server.listen(4)
    server.settimeout(0.015) # 15ms para resposta ultrarrápida do loop
    print("Servidor Web pronto! Acesse http://192.168.4.1 no navegador")
    print("Memoria RAM livre:", gc.mem_free(), "bytes\n")

RESP_OK = b'HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n'
RESP_REDIRECT = b'HTTP/1.1 302 Found\r\nLocation: http://192.168.4.1/\r\nContent-Length: 0\r\nConnection: close\r\n\r\n'

conectado = False
last_blink = time.ticks_ms()
last_idle_anim = time.ticks_ms()
idle_step = 0
last_gc = time.ticks_ms()

while True:
    agora = time.ticks_ms()

    if dns:
        dns.handle()

    # Processa quadros da animação dos olhos (não-bloqueante)
    processar_animacao(agora)

    # Processa modo autônomo se ativado
    if modo_autonomo:
        processar_autonomo(agora)

    # Watchdog de segurança: se o sinal Wi-Fi cair enquanto move, para em 3.5s
    if last_move_time is not None and not modo_autonomo:
        if time.ticks_diff(agora, last_move_time) > WATCHDOG_MS:
            parar()

    # Coleta de lixo APENAS em repouso (evita micro-pausas durante a pilotagem)
    if last_move_time is None and time.ticks_diff(agora, last_gc) > 12000:
        gc.collect()
        last_gc = agora

    conn = None
    try:
        try:
            conn, addr = server.accept()
        except OSError:
            # Tempo ocioso: animação não-bloqueante apenas se estiver parado
            if not modo_autonomo and last_move_time is None:
                if not conectado:
                    if time.ticks_diff(agora, last_idle_anim) > 800:
                        tela_espera()
                        last_idle_anim = agora
                else:
                    if time.ticks_diff(agora, last_blink) > 3200:
                        piscar_olhos(duplo=(idle_step % 3 == 0))
                        last_blink = agora
                    elif time.ticks_diff(agora, last_idle_anim) > 4200:
                        idle_step = (idle_step + 1) % 6
                        if idle_step == 0:
                            olhar_suave(-14, -2, "NORMAL", -3, 3)  # Olha curioso para a DIREITA (-14)
                        elif idle_step == 1:
                            olhar_suave(0, 0, "NORMAL", 0, 0)      # Volta ao centro
                            piscar_olhos()
                        elif idle_step == 2:
                            olhar_suave(14, -2, "NORMAL", 3, -3)   # Olha curioso para a ESQUERDA (+14)
                        elif idle_step == 3:
                            olhar_suave(0, -5, "NORMAL", 0, 0)     # Olha para cima pensativo
                        elif idle_step == 4:
                            desenha_olhos(0, -2, 0, 0, "FELIZ", 2, 2)
                            piscar_olhos(duplo=True)
                        else:
                            piscar_um_olho()
                        last_idle_anim = agora
            continue

        # --- REQUISIÇÃO HTTP RECEBIDA ---
        conn.settimeout(0.5)
        raw = conn.recv(512)
        if not raw:
            conn.close()
            continue

        try:
            req_line = raw.decode('utf-8').split('\r\n')[0]
            path = req_line.split()[1]
            q = path.find('?')
            if q != -1: path = path[:q]
        except Exception:
            path = "/"

        # Rota de alternância do Passeio Autônomo
        if path == "/autonomo":
            conectado = True
            modo_autonomo = not modo_autonomo
            if modo_autonomo:
                auto_proximo_tempo = agora
                auto_passo = 0
            else:
                parar()
            resp = b'{"autonomo":' + (b'true}' if modo_autonomo else b'false}')
            hdr = b'HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: ' + str(len(resp)).encode() + b'\r\nConnection: close\r\n\r\n'
            conn.sendall(hdr + resp)
            conn.close()
            conn = None
            continue

        # Comandos de Direção com resposta em < 5ms
        is_action = True
        proximos_olhos = None

        if path == "/frente":
            conectado = True
            modo_autonomo = False
            frente()
            proximos_olhos = (0, -3, 0, 0, "FOCADO", 0, 0)
        elif path == "/tras":
            conectado = True
            modo_autonomo = False
            tras()
            proximos_olhos = (0, 4, 0, 0, "SURPRESO", 0, 0)
        elif path == "/esquerda":
            conectado = True
            modo_autonomo = False
            esquerda()
            proximos_olhos = (16, 0, 0, 0, "NORMAL", 4, -4)   # Olhos olham para a ESQUERDA (+16 no display)
        elif path == "/direita":
            conectado = True
            modo_autonomo = False
            direita()
            proximos_olhos = (-16, 0, 0, 0, "NORMAL", -4, 4)  # Olhos olham para a DIREITA (-16 no display)
        elif path == "/frente_esq":
            conectado = True
            modo_autonomo = False
            frente_esq()
            proximos_olhos = (12, -3, 0, 0, "FOCADO", 3, -3)
        elif path == "/frente_dir":
            conectado = True
            modo_autonomo = False
            frente_dir()
            proximos_olhos = (-12, -3, 0, 0, "FOCADO", -3, 3)
        elif path == "/tras_esq":
            conectado = True
            modo_autonomo = False
            tras_esq()
            proximos_olhos = (12, 4, 0, 0, "SURPRESO", 3, -3)
        elif path == "/tras_dir":
            conectado = True
            modo_autonomo = False
            tras_dir()
            proximos_olhos = (-12, 4, 0, 0, "SURPRESO", -3, 3)
        elif path == "/parar":
            modo_autonomo = False
            if conectado:
                parar()
            proximos_olhos = (0, 0, 0, 0, "NORMAL", 0, 0)
        elif path == "/feliz":
            conectado = True
            proximos_olhos = (0, -3, 0, 0, "FELIZ", 2, 2)
            beep(880, 50)
        elif path == "/piscar":
            conectado = True
            piscar_um_olho()
        elif path == "/bip":
            conectado = True
            beep(988, 80)
        else:
            is_action = False

        if is_action:
            # 1. Envia resposta HTTP 200 OK e fecha socket imediatamente (< 1ms)!
            conn.sendall(RESP_OK)
            conn.close()
            conn = None
            # 2. Cancela qualquer animação ociosa pendente
            anim_queue = []
            # 3. Atualiza os olhos no OLED com os novos valores
            if proximos_olhos:
                desenha_olhos(*proximos_olhos)
            continue

        # Portal cativo de smartphones
        if path in ("/hotspot-detect.html", "/canonical.html", "/generate_204", "/gen_204", "/ncsi.txt", "/connecttest.txt"):
            conn.sendall(RESP_REDIRECT)
            conn.close()
            conn = None
            continue

        if path == "/favicon.ico":
            conn.sendall(b'HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n')
            conn.close()
            conn = None
            continue

        # Envia a página HTML do controle
        conectado = True
        hdr = ('HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %d\r\nConnection: close\r\n\r\n' % HTML_LEN).encode('utf-8')
        conn.sendall(hdr)
        conn.sendall(HTML_BYTES)
        conn.close()
        conn = None

    except Exception as e:
        if conn:
            try: conn.close()
            except Exception: pass
