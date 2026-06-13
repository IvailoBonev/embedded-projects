let socket = null,
  playerName = "",
  reconnectTimer = null,
  pingTimer = null;
const messageHandlers = {};

export function on(type, fn) {
  messageHandlers[type] = fn;
}

export function send(obj) {
  if (!socket || socket.readyState !== 1) return;
  socket.send(JSON.stringify(obj));
}

export function connect(name) {
  playerName = name;
  clearTimeout(reconnectTimer);

  const proto = location.protocol === "https:" ? "wss" : "ws";
  socket = new WebSocket(`${proto}://${location.host}/ws`);

  socket.onopen = () => {
    socket.send(JSON.stringify({ type: "join", name }));
    clearInterval(pingTimer);
    pingTimer = setInterval(
      () => socket.readyState === 1 && socket.send('{"type":"ping"}'),
      5000,
    );
    messageHandlers._connected?.();
  };

  socket.onclose = () => {
    clearInterval(pingTimer);
    messageHandlers._disconnected?.();
    reconnectTimer = setTimeout(() => connect(playerName), 3000);
  };

  socket.onerror = () => socket.close();

  socket.onmessage = (e) => {
    try {
      const msg = JSON.parse(e.data);
      if (msg.type !== "pong") messageHandlers[msg.type]?.(msg);
    } catch (_) {}
  };
}
