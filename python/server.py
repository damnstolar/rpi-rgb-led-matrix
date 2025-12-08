#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Serwer HTTP do kontrolowania matrycy LED za pomocą binarek z examples-api-use i utils.
Używa subprocess do uruchamiania text-scroller i led-image-viewer.
"""

import os
import signal
import subprocess
from flask import Flask, request, render_template_string

app = Flask(__name__)

# Ścieżki do binarek (zakładamy skompilowane)
TEXT_SCROLLER = "./examples-api-use/scrolling-text-example"
LED_IMAGE_VIEWER = "./utils/led-image-viewer"
FONT_PATH = "fonts/7x13.bdf"
GIF_DIR = "gifs"

# Parametry matrycy
MATRIX_ARGS = [
    "--led-rows=32",
    "--led-cols=128",
    "--led-chain=2",
    "--led-gpio-mapping=adafruit-hat",
    "--led-slowdown-gpio=3"
]

# Aktualny proces wyświetlania
current_process = None

def stop_current_display():
    """Zatrzymaj aktualne wyświetlanie."""
    global current_process
    if current_process:
        print(f"Zatrzymuję proces PID: {current_process.pid}")
        try:
            current_process.terminate()
            current_process.wait(timeout=5)
            print("Proces zatrzymany gracefully")
        except subprocess.TimeoutExpired:
            current_process.kill()
            print("Proces zabity forcefully")
        current_process = None
    else:
        print("Brak aktywnego procesu do zatrzymania")

@app.route('/')
def index():
    """Strona główna z interfejsem webowym."""
    gif_files = [f for f in os.listdir(GIF_DIR) if f.endswith(('.gif', '.jpg', '.png'))] if os.path.exists(GIF_DIR) else []
    return render_template_string("""
<!DOCTYPE html>
<html>
<head>
    <title>Kontrola Matrycy LED</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        form { margin-bottom: 20px; border: 1px solid #ccc; padding: 10px; }
        input, select, button { margin: 5px; padding: 8px; }
    </style>
</head>
<body>
    <h1>Kontrola Matrycy LED</h1>

    <form action="/text" method="post">
        <h2>Wyświetl tekst</h2>
        <input type="text" name="text" placeholder="Tekst do wyświetlenia" required>
        <input type="text" name="color" placeholder="Kolor RGB (np. 255,255,0)" value="255,255,0">
        <input type="number" name="speed" step="0.1" placeholder="Prędkość (1-10)" value="7.0" min="0.1" max="20">
        <button type="submit">Wyświetl tekst</button>
    </form>

    <form action="/gif" method="post">
        <h2>Wyświetl GIF/Obraz</h2>
        <select name="file" required>
            {% for gif in gif_files %}
            <option value="{{ gif }}">{{ gif }}</option>
            {% endfor %}
        </select>
        <button type="submit">Wyświetl</button>
    </form>

    <form action="/stop" method="post">
        <h2>Zatrzymaj wyświetlanie</h2>
        <button type="submit">Stop</button>
    </form>
</body>
</html>
    """, gif_files=gif_files)

@app.route('/text', methods=['POST'])
def display_text():
    """Wyświetl przewijający się tekst."""
    text = request.form.get('text', '')
    color = request.form.get('color', '255,255,0')
    speed = request.form.get('speed', '7.0')

    if not text:
        return "Brak tekstu", 400

    stop_current_display()

    cmd = [
        TEXT_SCROLLER,
        "-f", FONT_PATH,
        "-C", color,
        "-s", speed,
        "--led-no-hardware-pulse"
    ] + MATRIX_ARGS + [text]

    print(f"Uruchamiam komendę tekst: {' '.join(cmd)}")
    global current_process
    current_process = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"Uruchomiono proces tekst PID: {current_process.pid}")

    return "Wyświetlanie tekstu rozpoczęte"

@app.route('/gif', methods=['POST'])
def display_gif():
    """Wyświetl GIF lub obraz."""
    filename = request.form.get('file', '')
    filepath = os.path.join(GIF_DIR, filename)

    if not os.path.exists(filepath):
        return "Plik nie istnieje", 404

    stop_current_display()

    cmd = [
        LED_IMAGE_VIEWER,
        "--led-no-hardware-pulse"
    ] + MATRIX_ARGS + [filepath]

    print(f"Uruchamiam komendę GIF: {' '.join(cmd)}")
    global current_process
    current_process = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"Uruchomiono proces GIF PID: {current_process.pid}")

    return f"Wyświetlanie {filename} rozpoczęte"

@app.route('/stop', methods=['POST'])
def stop_display():
    """Zatrzymaj wyświetlanie."""
    stop_current_display()
    print("Wyświetlanie zatrzymane przez użytkownika")
    return "Wyświetlanie zatrzymane"

if __name__ == '__main__':
    print("Uruchamiam serwer HTTP na porcie 5000...")
    try:
        app.run(host='0.0.0.0', port=5000, debug=False)
    except KeyboardInterrupt:
        stop_current_display()
        print("Serwer zatrzymany.")