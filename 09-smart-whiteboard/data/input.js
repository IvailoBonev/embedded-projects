const $ = (id) => document.getElementById(id);
import { send } from "./ws.js";
import {
  canvas,
  toEsp,
  espSizeToPx,
  drawStroke,
  drawShape,
  drawText,
  snapshotToOffscreen,
  restoreFromOffscreen,
} from "./drawing.js";

export let tool = "pen",
  penColor = "#E74C3C",
  penSize = 4,
  shapeType = "rect";

let drawing = false,
  prevPos = null,
  shapeStart = null,
  textActive = false,
  textJustPlaced = false;

let strokePts = [];
const overlay = $("text-overlay");

export function setTool(t) {
  if (textActive) commitText();
  tool = t;
  updateCursor();
}
export function setPenColor(c) {
  penColor = c;
  if (textActive) overlay.style.color = c;
}
export function setPenSize(s) {
  penSize = s;
}
export function setShapeType(s) {
  shapeType = s;
}

function updateCursor() {
  canvas.style.cursor =
    tool === "eraser" ? "cell" : tool === "text" ? "text" : "crosshair";
}

function getPos(e) {
  const r = canvas.getBoundingClientRect();
  const s = e.touches?.[0] ?? e;
  return { x: s.clientX - r.left, y: s.clientY - r.top };
}

function sendChunk() {
  if (strokePts.length === 0) return;
  send({
    type: "stroke",
    points: strokePts,
    color: penColor,
    size: penSize,
    erase: tool === "eraser",
  });
}

function onPointerDown(e) {
  e.preventDefault();
  if (tool === "text") {
    if (textActive) commitText();
    placeText(getPos(e));
    return;
  }

  drawing = true;
  strokePts = [];
  prevPos = getPos(e);

  if (tool === "shape") {
    shapeStart = prevPos;
    snapshotToOffscreen();
    return;
  }

  const esp = toEsp(prevPos.x, prevPos.y);
  strokePts.push([esp.x, esp.y]);

  const color = tool === "eraser" ? "#fff" : penColor;
  const radius = espSizeToPx(tool === "eraser" ? 20 : penSize);
  drawStroke(prevPos.x, prevPos.y, prevPos.x, prevPos.y, color, radius);
}

function onPointerMove(e) {
  e.preventDefault();
  if (!drawing) return;

  const p = getPos(e);

  if (tool === "pen" || tool === "eraser") {
    const color = tool === "eraser" ? "#fff" : penColor;
    const radius = espSizeToPx(tool === "eraser" ? 20 : penSize);
    drawStroke(prevPos.x, prevPos.y, p.x, p.y, color, radius);

    const esp = toEsp(p.x, p.y);
    strokePts.push([esp.x, esp.y]);

    // Flush every 15 points to stay under ESP32's 512/1024 byte limit
    if (strokePts.length >= 15) {
      sendChunk();
      strokePts = [[esp.x, esp.y]];
    }
    prevPos = p;
  }

  if (tool === "shape" && shapeStart) {
    restoreFromOffscreen();
    drawShape(shapeType, shapeStart.x, shapeStart.y, p.x, p.y, penColor);
  }
}

function onPointerUp(e) {
  if (!drawing) return;
  drawing = false;

  if (tool === "shape" && shapeStart) {
    const p = getPos(e);
    restoreFromOffscreen();
    const start = toEsp(shapeStart.x, shapeStart.y);
    const end = toEsp(p.x, p.y);
    send({
      type: "shape",
      shape: shapeType,
      x1: start.x,
      y1: start.y,
      x2: end.x,
      y2: end.y,
      color: penColor,
    });
    shapeStart = null;
  } else {
    sendChunk();
    strokePts = [];
  }
  prevPos = null;
}

export function placeText(p) {
  overlay.style.left = p.x + "px";
  overlay.style.top = p.y + "px";
  overlay.style.color = penColor;
  overlay.style.display = "block";
  overlay.value = "";

  textActive = true;
  textJustPlaced = true;

  setTimeout(() => {
    textJustPlaced = false;
    overlay.focus();
  }, 50);
}

export function commitText() {
  if (!textActive) return;
  textActive = false;

  overlay.style.display = "none";
  const txt = overlay.value.trim();
  overlay.value = "";

  if (!txt) return;
  const x = parseFloat(overlay.style.left);
  const y = parseFloat(overlay.style.top) + 18;

  drawText(x, y, txt, penColor);

  const ep = toEsp(x, y);
  send({ type: "text", x: ep.x, y: ep.y, text: txt, color: penColor });
}

export function init() {
  canvas.addEventListener("pointerdown", onPointerDown);
  canvas.addEventListener("pointermove", onPointerMove, { passive: false });
  canvas.addEventListener("pointerup", onPointerUp);
  canvas.addEventListener("pointercancel", onPointerUp);

  overlay.addEventListener("keydown", (e) => {
    if (e.key === "Enter") {
      e.preventDefault();
      commitText();
    }
    if (e.key === "Escape") {
      textActive = false;
      overlay.style.display = "none";
      overlay.value = "";
    }
  });

  document.addEventListener("pointerdown", (e) => {
    if (textActive && !textJustPlaced && e.target !== overlay) commitText();
  });

  updateCursor();
}
