const $ = (id) => document.getElementById(id);

import { setTool, setPenColor, setPenSize, setShapeType } from "./input.js";
import { clearCanvas, canvas } from "./drawing.js";
import { send } from "./ws.js";

export function init() {
  // ── Tool buttons ───────────────────────────────────────
  document.querySelectorAll(".mc-tool").forEach((btn) => {
    btn.addEventListener("click", () => {
      const toolName = btn.dataset.tool;
      setTool(toolName);

      document
        .querySelectorAll(".mc-tool")
        .forEach((b) => b.classList.remove("active"));
      btn.classList.add("active");

      // Show shape selector only for shape tool
      const shapeSelector = $("shape-select");
      shapeSelector.classList.toggle("hidden", toolName !== "shape");
    });
  });

  // ── Shape selector ────────────────────────────────────
  $("shape-select").addEventListener("change", (e) => {
    setShapeType(e.target.value);
  });

  //Color swatches
  document.querySelectorAll(".mc-swatch").forEach((sw) => {
    sw.addEventListener("click", () => {
      setPenColor(sw.dataset.hex);
      document
        .querySelectorAll(".mc-swatch")
        .forEach((s) => s.classList.remove("active"));
      sw.classList.add("active");
      document.getElementById("custom-color").value = sw.dataset.hex;
    });
  });

  //Custom color picker
  $("custom-color").addEventListener("input", (e) => {
    setPenColor(e.target.value);
    document
      .querySelectorAll(".mc-swatch")
      .forEach((s) => s.classList.remove("active"));
  });

  //Size slider
  const sizeSlider = $("size-slider");
  const sizeVal = $("size-val");
  const sizeDot = $("size-dot");

  function updateSize(size) {
    setPenSize(size);
    sizeVal.textContent = String(size).padStart(2, "0");

    // Scale dot: size 1 = 4px, size 20 = 24px
    const px = 4 + ((size - 1) / 19) * 20;
    sizeDot.style.setProperty("--sz", px + "px");
  }

  sizeSlider.addEventListener("input", () =>
    updateSize(parseInt(sizeSlider.value)),
  );
  updateSize(4); // initial

  //Clear
  $("clear-btn").addEventListener("click", () => {
    if (!confirm("Clear the canvas for everyone?")) return;
    clearCanvas();
    send({ type: "clear" });
  });

  //Export PNG
  $("export-btn").addEventListener("click", () => {
    const downloadLink = document.createElement("a");
    downloadLink.download = "drawboard.png";
    downloadLink.href = canvas.toDataURL("image/png");
    downloadLink.click();
  });
}
