from flask import Flask, request, jsonify
import os
from werkzeug.utils import secure_filename

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = 'gifs'
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024  # 16MB limit

# Upewnij się, że katalog istnieje
os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)

@app.route('/')
def index():
    with open('index.html', 'r') as f:
        html = f.read()
    gifs = [f for f in os.listdir('gifs') if f.endswith(('.gif', '.GIF'))]
    # Zastąp całą sekcję select
    select_start = html.find('<select name="gif" id="gifSelect" required>')
    select_end = html.find('</select>', select_start) + 9
    if select_start != -1 and select_end != -1:
        gifs_options = '<option value="">Wybierz GIF</option>' + ''.join(f'<option value="{gif}">{gif}</option>' for gif in gifs)
        html = html[:select_start] + f'<select name="gif" id="gifSelect" required>{gifs_options}</select>' + html[select_end:]
    return html

@app.route('/set_text', methods=['POST'])
def set_text():
    text = request.form.get('text', 'Witaj!')
    with open('/tmp/display_mode.txt', 'w') as f:
        f.write(f"text:{text}")
    return jsonify({'status': 'ok', 'message': f'Tekst ustawiony: {text}'})

@app.route('/set_gif', methods=['POST'])
def set_gif():
    gif_name = request.form.get('gif')
    if gif_name and os.path.exists(os.path.join('gifs', gif_name)):
        with open('/tmp/display_mode.txt', 'w') as f:
            f.write(f"gif:gifs/{gif_name}")
        return jsonify({'status': 'ok', 'message': f'GIF ustawiony: {gif_name}'})
    return jsonify({'status': 'error', 'message': 'GIF nie znaleziony'})

@app.route('/upload_gif', methods=['POST'])
def upload_gif():
    if 'file' not in request.files:
        return jsonify({'status': 'error', 'message': 'Brak pliku'})
    file = request.files['file']
    if file.filename == '':
        return jsonify({'status': 'error', 'message': 'Brak nazwy pliku'})
    if file and file.filename.lower().endswith('.gif'):
        filename = secure_filename(file.filename)
        file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
        return jsonify({'status': 'ok', 'message': f'GIF przesłany: {filename}'})
    return jsonify({'status': 'error', 'message': 'Nieprawidłowy plik GIF'})

@app.route('/set_center', methods=['POST'])
def set_center():
    center = request.form.get('center') == 'true'
    with open('/tmp/display_mode.txt', 'w') as f:
        f.write(f"center:{center}")
    return jsonify({'status': 'ok', 'message': f'Centruj: {center}'})

@app.route('/set_brightness', methods=['POST'])
def set_brightness():
    bright = int(request.form.get('brightness', 100))
    with open('/tmp/display_mode.txt', 'w') as f:
        f.write(f"brightness:{bright}")
    return jsonify({'status': 'ok', 'message': f'Jasność: {bright}'})

@app.route('/get_gifs')
def get_gifs():
    gifs = [f for f in os.listdir('gifs') if f.endswith(('.gif', '.GIF'))]
    return jsonify(gifs)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=True)