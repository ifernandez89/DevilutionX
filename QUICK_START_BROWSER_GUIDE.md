# 🎮 Quick Start Guide - Diablo 1 Browser Edition

## 🚀 How to Run the Browser Edition

### **Method 1: Using the Batch File (Easiest)**
1. Double-click `start_browser_server.bat`
2. The script will automatically start a web server
3. Open your browser and go to the URL shown (usually `http://localhost:8000`)
4. Navigate to `nightmare_browser_WORKING_FINAL.html`

### **Method 2: Manual Python Server**
```bash
# Open command prompt in the DevilutionX folder
python -m http.server 8000

# Then open browser to:
# http://localhost:8000/nightmare_browser_WORKING_FINAL.html
```

### **Method 3: Using Node.js**
```bash
# Install http-server globally
npm install -g http-server

# Start server
http-server -p 8080

# Then open browser to:
# http://localhost:8080/nightmare_browser_WORKING_FINAL.html
```

### **Method 4: VS Code Live Server**
1. Open the DevilutionX folder in VS Code
2. Install "Live Server" extension
3. Right-click on `nightmare_browser_WORKING_FINAL.html`
4. Select "Open with Live Server"

## 🎯 Why You Need a Web Server

Browser security prevents loading WebAssembly files directly from `file://` URLs. You need an HTTP server to:
- Load WebAssembly (.wasm) files
- Access the game data (.data) files  
- Handle the MPQ file properly
- Enable all browser features

## 🎮 Once the Server is Running

1. **Open the URL** in your browser (Chrome, Firefox, Safari, Edge)
2. **Select your diablodat.mpq** file when prompted
3. **Wait for loading** (automatic validation and setup)
4. **Click START DIABLO 1** or wait for auto-start
5. **Play Diablo 1** directly in your browser!

## 🔧 Troubleshooting

### **If you see "404 Not Found":**
- Make sure you're using `http://localhost:PORT/` not `file://`
- Check that the web server is running
- Verify the HTML file exists in the same folder

### **If WebAssembly fails to load:**
- Ensure `devilutionx.wasm` and `devilutionx.data` are in the same folder
- Check browser console for error messages
- Try a different browser (Chrome recommended)

### **If MPQ validation fails:**
- Make sure you're selecting the correct `diablodat.mpq` file
- File should be 400-700MB in size
- Try a different copy of the MPQ file

## 📱 Mobile/Tablet Support

The browser edition works on mobile devices too:
- **iOS Safari**: Full support with touch controls
- **Android Chrome**: Full support with touch controls
- **Tablets**: Optimized interface for larger screens

## 🎯 Ready to Play!

Once you have the web server running, you'll have **Diablo 1 running directly in your browser** - exactly what you wanted for true portability!