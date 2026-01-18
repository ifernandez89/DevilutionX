const path = require('path'); 
const server = http.createServer((req, res) =
  const filePath = path.join(__dirname, req.url === '/' ? 'index.html' : req.url); 
  fs.readFile(filePath, (err, data) =
    if (err) { res.writeHead(404); res.end('Not found'); return; } 
    res.writeHead(200); res.end(data); 
  }); 
}); 
server.listen(8000, () = running on http://localhost:8000')); 
