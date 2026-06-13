import { send } from "./ws.js";

const usersList = document.getElementById("users-list");
const chatMsgs = document.getElementById("chat-messages");
const chatInput = document.getElementById("chat-input");
const chatSend = document.getElementById("chat-send");
const menuBtn = document.getElementById("menu-btn");
const sidebar = document.getElementById("sidebar");

let playerName = "";

export function init(name) {
  playerName = name;

  chatSend.addEventListener("click", sendChat);
  chatInput.addEventListener("keydown", (e) => {
    if (e.key === "Enter") sendChat();
  });
  menuBtn.addEventListener("click", () => sidebar.classList.toggle("open"));

  document.getElementById("canvas-wrap").addEventListener("pointerdown", () => {
    sidebar.classList.remove("open");
  });
}

function sendChat() {
  const txt = chatInput.value.trim();
  if (!txt) return;
  send({ type: "chat", text: txt });
  chatInput.value = "";
}

export function renderUsers(users) {
  usersList.innerHTML = "";
  users.forEach((u) => {
    const div = document.createElement("div");
    div.className = "user-chip";
    div.innerHTML = `
      <div class="user-dot" style="--c:${u.color}"></div>
      <span class="user-name">${u.name}</span>
      ${u.name === playerName ? '<span class="user-you">you</span>' : ""}`;
    usersList.appendChild(div);
  });
}

export function appendChat(name, color, text) {
  const div = document.createElement("div");

  div.className = "chat-msg";
  div.innerHTML = `
    <span class="chat-who" style="--c:${color}">${name}:</span>
    <span class="chat-text">${text}</span>`;

  chatMsgs.appendChild(div);

  // Auto-scroll to bottom
  chatMsgs.scrollTop = chatMsgs.scrollHeight;
}
