from flask import Flask, request, jsonify, send_from_directory
import subprocess
import os
import signal
import json

app = Flask(__name__)
CURRENT_PROCESS = None

CONFIG_FILE = "config.json"

def load_config():
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE) as f:
            return json.load(f)
    return {
        "brightness": 50,
        "source_dir": "media/gifs",
        "slowdown": 4,
        "led_rows": 32,
        "led_cols": 128,
        "led_gpio_mapping": "adafruit-hat"
    }

def save_config(cfg):
    with open(CONFIG_FILE, "w") as f:
        json.dump(cfg, f)

config = load_config()

def stop_current():
    global CURRENT_PROCESS
    if CURRENT_PROCESS:
        try:
            CURRENT_PROCESS.send_signal(signal.SIGTERM)
            CURRENT_PROCESS.wait(timeout=5)  # Wait for process to terminate
        except (OSError, subprocess.TimeoutExpired):
            try:
                CURRENT_PROCESS.kill()  # Force kill if needed
            except OSError:
                pass
        CURRENT_PROCESS = None

@app.route("/")
def index():
    return send_from_directory("static", "index.html")

@app.route("/play_folder", methods=["POST"])
def play_folder():
    stop_current()
    folder = request.json.get("folder")
    if not folder:
        return jsonify({"error": "Folder name required"}), 400
    folder_path = os.path.join(config["source_dir"], folder)
    if not os.path.isdir(folder_path):
        return jsonify({"error": "Folder does not exist"}), 400
    brightness = config["brightness"]

    cmd = [
        "sudo", "../utils/led-image-viewer",
        "-C", "-f", "-w15", "-t15", "-D80",
        os.path.join(folder_path, "*"),
        f"--led-rows={config['led_rows']}",
        f"--led-cols={config['led_cols']}",
        f"--led-gpio-mapping={config['led_gpio_mapping']}",
        f"--led-brightness={brightness}",
        f"--led-slowdown-gpio={config['slowdown']}"
    ]

    try:
        global CURRENT_PROCESS
        CURRENT_PROCESS = subprocess.Popen(cmd)
        return jsonify({"status": "ok"})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route("/play_single", methods=["POST"])
def play_single():
    stop_current()
    gif_path = request.json.get("path")
    if not gif_path:
        return jsonify({"error": "File path required"}), 400
    full_path = os.path.join(config["source_dir"], gif_path)
    if not os.path.isfile(full_path):
        return jsonify({"error": "File does not exist"}), 400
    brightness = config["brightness"]

    cmd = [
        "sudo", "../utils/led-image-viewer",
        "-C", "-f", "-t15", "-D80",
        full_path,
        f"--led-rows={config['led_rows']}",
        f"--led-cols={config['led_cols']}",
        f"--led-gpio-mapping={config['led_gpio_mapping']}",
        f"--led-brightness={brightness}",
        f"--led-slowdown-gpio={config['slowdown']}"
    ]

    try:
        global CURRENT_PROCESS
        CURRENT_PROCESS = subprocess.Popen(cmd)
        return jsonify({"status": "ok"})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route("/brightness", methods=["POST"])
def brightness():
    try:
        value = int(request.json["value"])
        if not 0 <= value <= 100:
            return jsonify({"error": "Brightness must be between 0 and 100"}), 400
        config["brightness"] = value
        save_config(config)
        return jsonify({"brightness": value})
    except (ValueError, KeyError):
        return jsonify({"error": "Invalid brightness value"}), 400

@app.route("/slowdown", methods=["POST"])
def slowdown():
    try:
        value = int(request.json["value"])
        if not -1 <= value <= 4:
            return jsonify({"error": "Slowdown must be between -1 and 4"}), 400
        config["slowdown"] = value
        save_config(config)
        return jsonify({"slowdown": value})
    except (ValueError, KeyError):
        return jsonify({"error": "Invalid slowdown value"}), 400

@app.route("/scroll_text", methods=["POST"])
def scroll_text():
    stop_current()
    text = request.json.get("text", "").strip()
    if not text:
        return jsonify({"error": "Text required"}), 400

    cmd = [
        "sudo", "../examples-api-use/scrolling-text-example",
        "-f", "../fonts/7x13.bdf",
        f"--led-rows={config['led_rows']}",
        f"--led-cols={config['led_cols']}",
        f"--led-gpio-mapping={config['led_gpio_mapping']}",
        f"--led-brightness={config['brightness']}",
        text
    ]

    try:
        global CURRENT_PROCESS
        CURRENT_PROCESS = subprocess.Popen(cmd)
        return jsonify({"status": "ok"})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route("/stop", methods=["POST"])
def stop():
    stop_current()
    return jsonify({"status": "stopped"})

@app.route("/set_source_dir", methods=["POST"])
def set_source_dir():
    new_dir = request.json.get("source_dir")
    if not new_dir or not os.path.isdir(new_dir):
        return jsonify({"error": "Invalid or non-existent directory"}), 400
    config["source_dir"] = new_dir
    save_config(config)
    return jsonify({"source_dir": new_dir})

@app.route("/list_files")
def list_files():
    try:
        files = os.listdir(config["source_dir"])
        return jsonify({"files": files})
    except OSError as e:
        return jsonify({"error": str(e)}), 500

@app.route("/upload", methods=["POST"])
def upload():
    if 'file' not in request.files:
        return jsonify({"error": "No file part"}), 400
    file = request.files['file']
    if file.filename == '':
        return jsonify({"error": "No selected file"}), 400
    if file and file.filename.lower().endswith(('.gif', '.jpg', '.jpeg', '.png')):
        filename = os.path.basename(file.filename)
        filepath = os.path.join(config["source_dir"], filename)
        file.save(filepath)
        return jsonify({"message": "File uploaded successfully", "filename": filename})
    else:
        return jsonify({"error": "Invalid file type. Only GIF, JPG, PNG allowed"}), 400

@app.route("/shuffle_folder", methods=["POST"])
def shuffle_folder():
    stop_current()
    import random
    try:
        files = [f for f in os.listdir(config["source_dir"]) if os.path.isfile(os.path.join(config["source_dir"], f))]
        random.shuffle(files)
        file_paths = [os.path.join(config["source_dir"], f) for f in files]
        if not file_paths:
            return jsonify({"error": "No files in directory"}), 400
    except OSError:
        return jsonify({"error": "Cannot access directory"}), 500

    brightness = config["brightness"]

    cmd = [
        "sudo", "../utils/led-image-viewer",
        "-C", "-f", "-w15", "-t15", "-D80"
    ] + file_paths + [
        f"--led-rows={config['led_rows']}",
        f"--led-cols={config['led_cols']}",
        f"--led-gpio-mapping={config['led_gpio_mapping']}",
        f"--led-brightness={brightness}",
        f"--led-slowdown-gpio={config['slowdown']}"
    ]

    try:
        global CURRENT_PROCESS
        CURRENT_PROCESS = subprocess.Popen(cmd)
        return jsonify({"status": "ok"})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
