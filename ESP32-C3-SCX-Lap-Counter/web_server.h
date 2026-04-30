// HTML embebido
const char* webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=yes">
    <title>Contador Vueltas</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            min-height: 100vh;
            /* Patrón de bandera de cuadros (checkered flag) */
            background-image: 
                linear-gradient(45deg, #000 25%, transparent 25%),
                linear-gradient(-45deg, #000 25%, transparent 25%),
                linear-gradient(45deg, transparent 75%, #000 75%),
                linear-gradient(-45deg, transparent 75%, #000 75%);
            background-size: 160px 160px;
            background-position: 0 0, 0 80px, 80px -80px, -80px 0px;
            background-color: #fff;
        }
        
        .container {
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            /* Fondo semitransparente para leer el texto */
            background-color: rgba(0, 0, 0, 0.75);
            min-height: 100vh;
            backdrop-filter: blur(2px);
        }
        
        /* Estado principal */
        .status {
            font-size: 52px;
            font-weight: bold;
            text-align: center;
            margin: 20px 0;
            padding: 25px;
            background: rgba(15, 52, 96, 0.9);
            border-radius: 15px;
            letter-spacing: 2px;
            color: white;
            border: 2px solid #ffaa00;
        }
        
        /* Mejor vuelta */
        .best {
            background: rgba(255, 0, 0, 0.9);
            padding: 20px;
            border-radius: 15px;
            font-size: 28px;
            font-weight: bold;
            text-align: center;
            margin: 20px 0;
            color: white;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
            border: 2px solid #ffaa00;
        }
        
        /* Lista de vueltas */
        .laps-container {
            margin: 20px 0;
        }
        
        .lap {
            font-size: 24px;
            margin: 10px 0;
            padding: 15px;
            background: rgba(22, 33, 62, 0.9);
            border-radius: 12px;
            text-align: center;
            font-weight: 500;
            color: white;
            border-left: 5px solid #ffaa00;
        }
        
        .lap-best {
            background: rgba(0, 170, 68, 0.9);
            color: white;
            border-left: 5px solid gold;
        }
        
        /* Sección de configuración */
        .config-section {
            background: rgba(15, 52, 96, 0.9);
            padding: 20px;
            border-radius: 15px;
            margin: 20px 0;
            text-align: center;
            border: 2px solid #ffaa00;
        }
        
        .config-section h3 {
            font-size: 28px;
            margin-bottom: 15px;
            color: white;
        }
        
        .config-section label {
            font-size: 22px;
            display: inline-block;
            margin: 10px;
            color: white;
        }
        
        .config-section input {
            font-size: 24px;
            width: 80px;
            text-align: center;
            padding: 8px;
            border-radius: 10px;
            border: none;
            margin: 0 5px;
        }
        
        /* Botones */
        .buttons {
            display: flex;
            gap: 15px;
            justify-content: center;
            margin: 20px 0;
            flex-wrap: wrap;
        }
        
        button {
            font-size: 32px;
            padding: 18px 30px;
            border: none;
            border-radius: 15px;
            cursor: pointer;
            font-weight: bold;
            transition: transform 0.2s, opacity 0.2s;
        }
        
        button:active {
            transform: scale(0.97);
        }
        
        .btn-start {
            background: #00ff88;
            color: #1a1a2e;
        }
        
        .btn-reset {
            background: #ff4444;
            color: white;
        }
        
        .btn-config {
            background: #ffaa00;
            color: #1a1a2e;
            font-size: 28px;
            padding: 12px 24px;
        }
        
        /* Pie de página */
        .footer {
            text-align: center;
            font-size: 16px;
            margin-top: 30px;
            color: #ccc;
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- Estado principal -->
        <div id="status" class="status">---</div>
        
        <!-- Mejor vuelta -->
        <div id="best" class="best">ESPERANDO...</div>
        
        <!-- Lista de vueltas -->
        <div id="laps" class="laps-container"></div>
        
        <!-- Configuración -->
        <div class="config-section">
            <h3>CONFIGURACION</h3>
            <label>Cuenta atras:</label>
            <input type="number" id="cd" value="3" min="1" max="10">
            <label>Vueltas:</label>
            <input type="number" id="lapsCount" value="3" min="1" max="30">
            <br />
            <br />
            <button class="btn-config" onclick="updateConfig()">APLICAR</button>
        </div>
        
        <!-- Botones de control -->
        <div class="buttons">
            <button class="btn-start" onclick="start()">INICIAR</button>
            <button class="btn-reset" onclick="reset()">REINICIAR</button>
        </div>
        
        <div class="footer">
            CONTADOR DE VUELTAS - BANDERA DE CUADROS
        </div>
    </div>
    
    <script>
        function fetchData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    // Actualizar estado
                    let stateText = '';
                    switch(data.state) {
                        case 'IDLE': stateText = 'LISTO'; break;
                        case 'COUNTDOWN': stateText = 'PREPARANDO'; break;
                        case 'RACING': stateText = 'CORRIENDO'; break;
                        case 'RACE_FINISHED': stateText = 'TERMINADO'; break;
                        default: stateText = data.state;
                    }
                    document.getElementById('status').innerHTML = stateText;
                    
                    // Actualizar lista de vueltas
                    let lapsHtml = '';
                    for(let i = 0; i < data.laps.length; i++) {
                        let bestClass = (data.best == data.laps[i]) ? 'lap-best' : '';
                        lapsHtml += `<div class="lap ${bestClass}">VUELTA ${i+1}: ${data.laps[i]} s</div>`;
                    }
                    document.getElementById('laps').innerHTML = lapsHtml;
                    
                    // Actualizar mejor vuelta
                    if(data.best > 0) {
                        document.getElementById('best').innerHTML = `MEJOR VUELTA: ${data.best} s`;
                    } else {
                        document.getElementById('best').innerHTML = 'ESPERANDO PRIMERA VUELTA...';
                    }
                })
                .catch(error => {
                    console.log('Error:', error);
                });
        }
        
        function start() {
            fetch('/start');
            setTimeout(fetchData, 100);
        }
        
        function reset() {
            fetch('/reset');
            setTimeout(fetchData, 100);
        }
        
        function updateConfig() {
            let cd = document.getElementById('cd').value;
            let laps = document.getElementById('lapsCount').value;
            fetch(`/config?cd=${cd}&laps=${laps}`);
            setTimeout(() => {
                fetchData();
                let btn = document.querySelector('.btn-config');
                btn.style.opacity = '0.5';
                setTimeout(() => { btn.style.opacity = '1'; }, 500);
            }, 200);
        }
        
        setInterval(fetchData, 500);
        fetchData();
    </script>
</body>
</html>
)rawliteral";

/*const char* webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Contador Vueltas</title>
    <style>
        body { font-family: Arial; text-align: center; margin: 20px; background: #1a1a2e; color: white; }
        .container { max-width: 600px; margin: auto; }
        .lap { font-size: 22px; margin: 8px; padding: 8px; background: #16213e; border-radius: 8px; }
        .best { background: red; padding: 12px; border-radius: 10px; font-size: 24px; margin: 15px 0; }
        button { font-size: 20px; margin: 10px; padding: 12px 24px; border: none; border-radius: 8px; cursor: pointer; }
        .btn-start { background: #00ff88; color: #1a1a2e; }
        .btn-reset { background: #ff4444; color: white; }
        .btn-config { background: #ffaa00; color: #1a1a2e; }
        input { font-size: 18px; width: 70px; text-align: center; padding: 5px; margin: 0 5px; }
        .status { font-size: 28px; margin: 20px 0; padding: 15px; background: #0f3460; border-radius: 10px; }
        h1 { color: #00ff88; }
        select, input { padding: 8px; font-size: 16px; border-radius: 5px; border: none; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🏁 CONTADOR DE VUELTAS 🏁</h1>
        <div id="status" class="status">---</div>
        <div id="laps"></div>
        <div id="best" class="best"></div>
        
        <div style="background: #0f3460; padding: 15px; border-radius: 10px; margin: 15px 0;">
            <h3>⚙️ CONFIGURACIÓN</h3>
            <label>⏱️ Cuenta atrás: 
                <input type="number" id="cd" value="3" min="1" max="10">
            </label>
            <label>🔄 Vueltas: 
                <input type="number" id="lapsCount" value="3" min="1" max="30">
            </label>
            <button class="btn-config" onclick="updateConfig()">Aplicar</button>
        </div>
        
        <div>
            <button class="btn-start" onclick="start()">🚦 INICIAR CARRERA</button>
            <button class="btn-reset" onclick="reset()">🔄 RESET</button>
        </div>
    </div>
    
    <script>
        function fetchData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('status').innerHTML = data.state;
                    let html = '';
                    for(let i = 0; i < data.laps.length; i++) {
                        let bg = (data.best == data.laps[i]) ? 'style="background:#00aa44"' : '';
                        html += `<div class="lap" ${bg}>🏁 Vuelta ${i+1}: ${data.laps[i]} s</div>`;
                    }
                    document.getElementById('laps').innerHTML = html;
                    if(data.best > 0) {
                        document.getElementById('best').innerHTML = `🏆 MEJOR VUELTA: ${data.best} s 🏆`;
                    } else {
                        document.getElementById('best').innerHTML = 'Esperando primera vuelta...';
                    }
                });
        }
        
        function start() {
            fetch('/start');
            setTimeout(fetchData, 100);
        }
        
        function reset() {
            fetch('/reset');
            setTimeout(fetchData, 100);
        }
        
        function updateConfig() {
            let cd = document.getElementById('cd').value;
            let laps = document.getElementById('lapsCount').value;
            fetch(`/config?cd=${cd}&laps=${laps}`);
            setTimeout(fetchData, 500);
            setTimeout(() => fetchData(), 1000);
        }
        
        setInterval(fetchData, 500);
        fetchData();
    </script>
</body>
</html>
)rawliteral";
*/