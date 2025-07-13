# ImageFilter

A web server written in C that applies filters to bitmap images. Upload a BMP image and apply filters like grayscale, edge detection, or Gaussian blur through a web interface.

## What it does

- Web server that processes bitmap image files
- Upload images via web form
- Apply image filters (copy, grayscale, gaussian blur, edge detection)
- Download processed images

## How to run

1. **Build the project**:
   ```bash
   make
   ```

2. **Start the server**:
   ```bash
   ./image_server
   ```

3. **Use the web interface**:
   Open your browser to: `http://localhost:30001/main.html`

## Requirements

- Linux/WSL environment
- GCC compiler (`sudo apt install build-essential`)

## Troubleshooting

- **"Address already in use"**: Run `pkill image_server` first
- **Can't access website**: Try `http://127.0.0.1:30001/main.html`
- **Missing executables**: Run `make` to rebuild filters

## License

This project is for educational purposes only and is not intended for production use.
