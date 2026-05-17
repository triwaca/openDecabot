String html = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Controle do Robô</title>
  <style>
    :root{
      --bg:#1a1a2e;
      --btn:#16213e;
      --border:#0f3460;
      --txt:#fff;
      --accent:#0f3460;
    }
    *{box-sizing:border-box}
    html,body{
      height:100%;
      margin:0;
    }
    body {
      background: var(--bg);
      font-family: Arial, Helvetica, sans-serif;
      color: var(--txt);
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      gap: 18px;
      user-select: none;
      -webkit-user-select: none;
    }
    h1{margin:0 0 4px 0;font-size:20px;opacity:.9}
    .controle {
      display: grid;
      grid-template-columns: 90px 90px 90px;
      grid-template-rows: 90px 90px 90px;
      gap: 14px;
      touch-action: none; /* evita scroll durante o toque */
    }
    button {
      background: var(--btn);
      color: var(--txt);
      border: 2px solid var(--border);
      border-radius: 16px;
      font-size: 22px;
      cursor: pointer;
      transition: transform .08s ease, background .15s ease;
      outline: none;
      -webkit-tap-highlight-color: transparent;
    }
    button:active{
      transform: scale(0.97);
      background: var(--accent);
    }
    /* posição dos botões */
    .frente  { grid-column: 2; grid-row: 1; }
    .esquerda{ grid-column: 1; grid-row: 2; }
    .direita { grid-column: 3; grid-row: 2; }
    .tras    { grid-column: 2; grid-row: 3; }

    .status{
      font-size:14px;
      opacity:.85;
    }

    @media (max-width: 420px){
      .controle{
        grid-template-columns: 80px 80px 80px;
        grid-template-rows: 80px 80px 80px;
        gap: 12px;
      }
    }
  </style>
</head>
<body>
  <h1>Controle do Robô</h1>

  <div class="controle">
    <button class="frente"   aria-label="Frente"   onpointerdown="press('Front')" onpointerup="release()" onpointerleave="release()">▲</button>
    <button class="esquerda" aria-label="Esquerda" onpointerdown="press('Left')"  onpointerup="release()" onpointerleave="release()">◄</button>
    <button class="direita"  aria-label="Direita"  onpointerdown="press('Right')" onpointerup="release()" onpointerleave="release()">►</button>
    <button class="tras"     aria-label="Trás"     onpointerdown="press('Back')"  onpointerup="release()" onpointerleave="release()">▼</button>
  </div>

  <div class="status">LED status: <strong id="ledState">unknown</strong></div>

  <script>
    let isPressed = false;

    function setLED(state) {
      return fetch('/led?state=' + state)
        .then(r => r.text())
        .then(text => {
          const el = document.getElementById('ledState');
          if (el) el.textContent = text;
        })
        .catch(() => {});
    }

    function press(direction){
      if (isPressed) return;
      isPressed = true;
      setLED(direction);
    }

    function release(){
      if (!isPressed) return;
      isPressed = false;
      setLED('Stop'); // volta para neutro ao SOLTAR
    }

    function getLEDStatus() {
      fetch('/status')
        .then(r => r.text())
        .then(text => {
          const el = document.getElementById('ledState');
          if (el) el.textContent = text;
        })
        .catch(() => {});
    }

    // Se sair da aba, soltaqqeqeqeqqeqqe
    window.addEventListener('blur', release);
    // Estado inicial
    window.onload = getLEDStatus;
  </script>
</body>
</html>
    )rawliteral";