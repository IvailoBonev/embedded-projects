const $ = (id) => document.getElementById(id);

import { connect, on, send } from "./ws.js";
import {
  canvas,
  resizeCanvas,
  fromEsp,
  espSizeToPx,
  drawStroke,
  drawShape,
  drawText,
  clearCanvas,
} from "./drawing.js";

import { init as initTools } from "./input.js";
import { init as initToolbar } from "./toolbar.js";
import { init as initSidebar, renderUsers, appendChat } from "./sidebar.js";

let playerName = "",
  playerColor = "#E74C3C";

//Join logic
$("join-btn").addEventListener("click", handleJoin);

$("join-name").addEventListener("keydown", (e) => {
  if (e.key === "Enter") handleJoin();
});

function handleJoin() {
  const name = $("join-name").value.trim();
  const err = $("join-error");

  if (name.length < 2) {
    err.textContent = "Name too short!";
    return;
  }
  err.textContent = "";
  playerName = name;

  launch();
}

function launch() {
  $("join-screen").style.display = "none";
  $("app").classList.remove("hidden");

  resizeCanvas();

  window.addEventListener("resize", resizeCanvas);

  initTools();
  initToolbar();
  initSidebar(playerName);

  handleSocket();
  connect(playerName);
}

function handleSocket() {
  on("_connected", () => {
    $("conn-dot").classList.add("online");
    $("conn-label").textContent = "online";
  });

  on("_disconnected", () => {
    $("conn-dot").classList.remove("online");
    $("conn-label").textContent = "offline";
  });

  on("joined", (msg) => {
    playerColor = msg.color;
  });

  on("reject", (msg) => {
    $("app").classList.add("hidden");
    $("join-screen").style.display = "flex";
    $("join-error").textContent = msg.reason || "Rejected";
  });

  on("users", (msg) => renderUsers(msg.users));

  on("stroke", (msg) => onStroke(msg));

  on("shape", (msg) => onShape(msg));

  on("text", (msg) => onText(msg));

  on("clear", () => clearCanvas());

  on("chat", (msg) => appendChat(msg.name, msg.color, msg.text));
}

function onStroke(msg) {
  const pts = msg.points;
  if (!pts || pts.length === 0) return;

  // Eraser logic: if erase is true, use white.
  const col = msg.erase ? "#ffffff" : msg.color || "#000000";
  const size = espSizeToPx(msg.erase ? 20 : msg.size || 4);

  if (pts.length === 1) {
    const p = fromEsp(pts[0][0], pts[0][1]);
    drawStroke(p.x, p.y, p.x, p.y, col, size);
  } else {
    for (let i = 1; i < pts.length; i++) {
      const p1 = fromEsp(pts[i - 1][0], pts[i - 1][1]);
      const p2 = fromEsp(pts[i][0], pts[i][1]);

      drawStroke(p1.x, p1.y, p2.x, p2.y, col, size);
    }
  }
}

function onShape(msg) {
  const p1 = fromEsp(msg.x1, msg.y1),
    p2 = fromEsp(msg.x2, msg.y2);
  drawShape(msg.shape, p1.x, p1.y, p2.x, p2.y, msg.color || "#000");
}

function onText(msg) {
  const p = fromEsp(msg.x, msg.y);
  drawText(p.x, p.y, msg.text, msg.color || "#000");
}
