const express = require("express");
const path = require("path");
const app = express();

// Get the port from the command-line arguments, or default to 3000
const PORT = process.argv[2] || 3000;

app.use(express.static(path.join(__dirname, "dist")));
app.use(express.static(path.join(__dirname, "src", "templates")));

app.listen(PORT, () => {
  console.log(`WebUI Dev Server running on http://localhost:${PORT}`);
});