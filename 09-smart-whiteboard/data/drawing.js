// ESP32 display canvas area: 480 wide, 270 tall (320 - 50px toolbar)
export const ESP_W = 480;
export const ESP_H = 270;

export const canvas = document.getElementById("canvas");
export const ctx = canvas.getContext("2d");

// Offscreen canvas for shape preview — avoids expensive getImageData
let offscreenCanvas = null;
let offscreenContext = null;
let _snapValid = false;

//Resize
export function resizeCanvas() {
  const wrap = document.getElementById("canvas-wrap");
  const w = wrap.clientWidth;
  const h = wrap.clientHeight;

  // Save pixels before resize wipes them
  let saved = null;
  if (canvas.width > 0 && canvas.height > 0) {
    saved = document.createElement("canvas");
    saved.width = canvas.width;
    saved.height = canvas.height;
    saved.getContext("2d").drawImage(canvas, 0, 0);
  }

  canvas.width = w;
  canvas.height = h;
  ctx.fillStyle = "#fff";
  ctx.fillRect(0, 0, w, h);

  if (saved) ctx.drawImage(saved, 0, 0, canvas.width, canvas.height);

  // Rebuild offscreen at new size
  offscreenCanvas = document.createElement("canvas");
  offscreenCanvas.width = w;
  offscreenCanvas.height = h;

  offscreenContext = offscreenCanvas.getContext("2d");
  _snapValid = false;
}

//Coordinate conversion
export function toEsp(cx, cy) {
  return {
    x: Math.round(
      Math.min(Math.max((cx / canvas.width) * ESP_W, 0), ESP_W - 1),
    ),
    y: Math.round(
      Math.min(Math.max((cy / canvas.height) * ESP_H, 0), ESP_H - 1),
    ),
  };
}

export function fromEsp(ex, ey) {
  return {
    x: (ex / ESP_W) * canvas.width,
    y: (ey / ESP_H) * canvas.height,
  };
}

export function espSizeToPx(size) {
  // Map pen size 1-20 to a reasonable pixel radius on screen
  return Math.max(1, Math.round((size / 20) * 16 + 1));
}

//Draw logic
export function drawStroke(x1, y1, x2, y2, color, radius) {
  ctx.beginPath();
  ctx.strokeStyle = color;
  ctx.lineWidth = radius * 2;
  ctx.lineCap = "round";
  ctx.lineJoin = "round";

  ctx.moveTo(x1, y1);
  ctx.lineTo(x2, y2);
  ctx.stroke();

  ctx.beginPath();
  ctx.fillStyle = color;
  ctx.arc(x2, y2, radius, 0, Math.PI * 2);
  ctx.fill();
}

export function drawShape(type, x1, y1, x2, y2, color, lineWidth) {
  ctx.strokeStyle = color;
  ctx.lineWidth = lineWidth || 2;
  ctx.beginPath();

  switch (type) {
    case "rect":
      ctx.strokeRect(
        Math.min(x1, x2),
        Math.min(y1, y2),
        Math.abs(x2 - x1),
        Math.abs(y2 - y1),
      );
      break;
    case "circle":
      ctx.ellipse(
        (x1 + x2) / 2,
        (y1 + y2) / 2,
        Math.abs(x2 - x1) / 2,
        Math.abs(y2 - y1) / 2,
        0,
        0,
        Math.PI * 2,
      );
      ctx.stroke();
      break;
    case "line":
      ctx.moveTo(x1, y1);
      ctx.lineTo(x2, y2);
      ctx.stroke();
      break;
  }
}

export function drawText(x, y, text, color) {
  const fs = 30;
  ctx.font = `bold ${fs}px monospace`;
  ctx.fillStyle = color;
  ctx.fillText(text, x, y);
}

export function clearCanvas() {
  ctx.fillStyle = "#fff";
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  _snapValid = false;
}

// ── Shape preview — uses offscreen copy, NOT getImageData ────
export function snapshotToOffscreen() {
  if (!offscreenCanvas) return;
  offscreenContext.clearRect(
    0,
    0,
    offscreenCanvas.width,
    offscreenCanvas.height,
  );
  offscreenContext.drawImage(canvas, 0, 0);
  _snapValid = true;
}

export function restoreFromOffscreen() {
  if (!offscreenCanvas || !_snapValid) return;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.drawImage(offscreenCanvas, 0, 0);
}
